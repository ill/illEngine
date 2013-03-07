#include "Graphics/serial/BitmapFont.h"
#include "Graphics/GraphicsBackend.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"

#include "Logging/logging.h"

const int32_t HEADER = 0x424D4603;   //"BMF" followed by format specifier 3 in big endian form

namespace illGraphics {

glm::mediump_float BitmapFont::getLineWidth(char const ** text) const {
    if(m_state != RES_LOADED) {
        LOG_FATAL_ERROR("Attempting to get line width of bitmap font when it's not loaded.");
    }

    glm::mediump_float curX = 0;

    //go through the string until a newline and get dimensions of every character
    while (**text && **text != '\n') {
        //check if color code
        if(getColorCode(text, glm::vec4())) {
            continue;
        }
        
        //parse special characters
        switch (**text) {
        case ' ': //space

            curX += m_spacingHorz;
            *text++;
            continue;

        case '\t': //tab

            curX += m_spacingHorz * 4.0f;
            *text++;
            continue;
        }

        curX += m_charData[(unsigned char) (**text)].m_advance;
        *text++;
    }
    
    return curX;
}

glm::vec2 BitmapFont::getPrintDimensions(const char * text) const {
    glm::mediump_float curX = 0;
    glm::mediump_float maxX = 0;
    glm::mediump_float curY = 0;

    //go through the string and get dimensions of every character
    while (*text) {
        curX = getLineWidth(&text);

        if (*text == '\n') {
            text++;
            curY += m_lineHeight;
        }

        if (curX > maxX) {
            maxX = curX;
        }
    }

    return glm::vec2(curX > maxX ? curX : maxX, curY + m_lineHeight);
}

bool BitmapFont::getColorCode(const char ** text, glm::vec4& destination) {
    return false;   //TODO
}

void BitmapFont::unload() {
    if(m_state == RES_LOADING) {
        LOG_FATAL_ERROR("Attempting to unload bitmap font while it's loading");
    }

    if(m_state == RES_UNINITIALIZED || m_state == RES_UNLOADED) {
        return;
    }

    m_mesh.unload();

    m_pageTextures.clear();
    m_kerningPairs.clear();

    m_state = RES_UNLOADED;
}

void BitmapFont::reload(GraphicsBackend * backend) {
    unload();
    
    m_loader = backend;

    m_state = RES_LOADING;

    illFileSystem::File * openFile = illFileSystem::fileSystem->openRead(m_loadArgs.m_path.c_str());

    /////////////////////////////
    //read header
    {
        uint32_t header;
        openFile->readB32(header);

        if(header != HEADER) {
            LOG_FATAL_ERROR("Font file %s is not a valid Angelcode Bitmap Font Generator binary Version 3 file", m_loadArgs.m_path.c_str());
        }
    }
    
    unsigned int textureWidth;
    unsigned int textureHeight;

    /////////////////////////////
    //read blocks
    while(!openFile->eof()) {
        //get block type
        uint8_t blockType;
        openFile->read8(blockType);

        //get block size
        uint32_t blockSize;
        openFile->readL32(blockSize);
        
        switch(blockType) {
        case 1:         //info
            readInfo(openFile, (size_t) blockSize);
            break;

        case 2:         //common
            readCommon(openFile, textureWidth, textureHeight);
            break;

        case 3:         //pages
            readPages(openFile, (size_t) blockSize);
            break;

        case 4:         //chars
            readChars(openFile, (size_t) blockSize, textureWidth, textureHeight);
            break;

        case 5:         //kerning pairs
            readKerningPairs(openFile, (size_t) blockSize);
            break;
        }
    }

    delete openFile;

    m_state = RES_LOADED;
}

void BitmapFont::readInfo(illFileSystem::File * file, size_t size) {
    //skip to the stuff I need
    file->seekAhead(7);

    //get padding
    uint8_t value;

    file->read8(value);
    m_paddingUp = (glm::mediump_float) value;

    file->read8(value);
    m_paddingDown = (glm::mediump_float) value;

    file->read8(value);
    m_paddingLeft = (glm::mediump_float) value;

    file->read8(value);
    m_paddingRight = (glm::mediump_float) value;

    //get spacing
    file->read8(value);
    m_spacingHorz = (glm::mediump_float) value;

    file->read8(value);
    m_spacingVert = (glm::mediump_float) value;

    //skip the rest of the block
    file->seekAhead(size - (7 + 6));
}

void BitmapFont::readCommon(illFileSystem::File * file, unsigned int& textureWidth, unsigned int& textureHeight) {
    //read line height
    {
        uint16_t lineHeight;
        file->readL16(lineHeight);

        m_lineHeight = (glm::mediump_float) lineHeight;
    }

    //read line base
    {
        uint16_t lineBase;
        file->readL16(lineBase);

        m_lineBase = (glm::mediump_float) lineBase;
    }

    //read the scale
    {
        uint16_t scale;

        file->readL16(scale);
        textureWidth = (unsigned int) scale;

        file->readL16(scale);
        textureHeight = (unsigned int) scale;
    }

    //read pages number
    {
        uint16_t pages;
        file->readL16(pages);

        m_pageTextures.resize((size_t) pages);
    }

    //TODO: read the bitfield
    file->seekAhead(5);
}

void BitmapFont::readPages(illFileSystem::File * file, size_t size) {
    //string length is the same for all file names in this block
    size_t pathSize = size / m_pageTextures.size();
    char * pathBuffer = new char[pathSize];

    //TODO: make the texture path be relative to the path of the font file

    TextureLoadArgs loadArgs;
    loadArgs.m_wrapS = TextureLoadArgs::Wrap::W_CLAMP_TO_EDGE;
    loadArgs.m_wrapT = TextureLoadArgs::Wrap::W_CLAMP_TO_EDGE;

    for(unsigned int texture = 0; texture < m_pageTextures.size(); texture++) {
        file->read(pathBuffer, pathSize);
        
        loadArgs.m_path.assign(pathBuffer);
        
        m_pageTextures[texture].load(loadArgs, m_loader);
    }

    delete[] pathBuffer;
}

void BitmapFont::readChars(illFileSystem::File * file, size_t size, unsigned int textureWidth, unsigned int textureHeight) {
    memset(m_charData, 0, sizeof(CharData) * NUM_CHARS);

    glm::mediump_float texW = 1.0f / textureWidth;
    glm::mediump_float texH = 1.0f / textureHeight;

    unsigned int numChars = (unsigned int) size / 20;
    
    //create a mesh data object with 2 triangles per character and 4 verteces per character to create quads
    m_mesh.unload();
    m_mesh.setFrontentDataInternal(new MeshData<>(numChars << 1, numChars << 2, MF_POSITION | MF_TEX_COORD));
    
    uint16_t * indeces = m_mesh.getMeshFrontentData()->getIndeces();

    for(unsigned int currChar = 0; currChar < numChars; currChar++) {
        unsigned char character;

        //read the character id
        {
            uint32_t ch;
            file->readL32(ch);

            assert(ch < NUM_CHARS);

            character = (unsigned char) ch;
        }

        //read the character size data
        {
            uint16_t val;

            file->readL16(val);
            glm::mediump_float left = (glm::mediump_float) val * texW;

            file->readL16(val);
            glm::mediump_float top = 1.0f - (glm::mediump_float) val * texH;

            file->readL16(val);
            glm::mediump_float width = (glm::mediump_float) val;

            file->readL16(val);
            glm::mediump_float height = (glm::mediump_float) val;

            glm::mediump_float right = left + width * texW;
            glm::mediump_float bottom = top - height * texH;

            file->readL16(val);
            glm::mediump_float xOffset = (glm::mediump_float) (int16_t) val;

            file->readL16(val);
            glm::mediump_float yOffset = (glm::mediump_float) (int16_t) val;

            file->readL16(val);
            m_charData[character].m_advance = (glm::mediump_float) (int16_t) val;

            /*
            A character's mesh looks like this
            vtx 3   _____   vtx 2
                    |   /
                    |  /|
                    | / |
                    |/  |
            vtx 0   /___|   vtx 1
            */

            //set the character index buffer data
            size_t firstIndex = currChar * 6;
            size_t firstVertex = currChar << 2;

            m_charData[character].m_meshIndex = (uint16_t) firstIndex;

            //tri 1
            indeces[firstIndex] = (uint16_t) firstVertex;                                                                               //vtx 0
            indeces[firstIndex + 1] = (uint16_t) firstVertex + 1;                                                                       //vtx 1
            indeces[firstIndex + 2] = (uint16_t) firstVertex + 2;                                                                       //vtx 2

            //tri 2
            indeces[firstIndex + 3] = (uint16_t) firstVertex + 2;                                                                       //vtx 2
            indeces[firstIndex + 4] = (uint16_t) firstVertex + 3;                                                                       //vtx 3
            indeces[firstIndex + 5] = (uint16_t) firstVertex;                                                                           //vtx 0

            //set the character vertex buffer data
            //positions
            m_mesh.getMeshFrontentData()->getPosition((uint32_t) firstVertex) = glm::vec3(xOffset, yOffset, 0.0f);                         //vtx 0
            m_mesh.getMeshFrontentData()->getPosition((uint32_t) firstVertex + 1) = glm::vec3(xOffset + width, yOffset, 0.0f);             //vtx 1
            m_mesh.getMeshFrontentData()->getPosition((uint32_t) firstVertex + 2) = glm::vec3(xOffset + width, yOffset + height, 0.0f);    //vtx 2
            m_mesh.getMeshFrontentData()->getPosition((uint32_t) firstVertex + 3) = glm::vec3(xOffset, yOffset + height, 0.0f);            //vtx 3

            //tex coords
            m_mesh.getMeshFrontentData()->getTexCoord((uint32_t) firstVertex) = glm::vec2(left, bottom);                                   //vtx 0
            m_mesh.getMeshFrontentData()->getTexCoord((uint32_t) firstVertex + 1) = glm::vec2(right, bottom);                              //vtx 1
            m_mesh.getMeshFrontentData()->getTexCoord((uint32_t) firstVertex + 2) = glm::vec2(right, top);                                 //vtx 2
            m_mesh.getMeshFrontentData()->getTexCoord((uint32_t) firstVertex + 3) = glm::vec2(left, top);                                  //vtx 3

            //int x = 5;
        }

        //read the texture page
        {
            uint8_t page;
            file->read8(page);

            m_charData[character].m_texturePage = page;
        }

        //TODO: read the channel
        file->seekAhead(1);
    }

    for(unsigned int ind = 0; ind < m_mesh.getMeshFrontentData()->getNumTri() * 3; ind++) {
        LOG_DEBUG("ind %u %u", ind, indeces[ind]);
    }

    for(unsigned int ind = 0; ind < m_mesh.getMeshFrontentData()->getNumVert(); ind++) {
        LOG_DEBUG("vert %u %f %f %f", ind, m_mesh.getMeshFrontentData()->getPosition(ind).x, m_mesh.getMeshFrontentData()->getPosition(ind).y, m_mesh.getMeshFrontentData()->getPosition(ind).z);
    }

    for(unsigned int ind = 0; ind < m_mesh.getMeshFrontentData()->getNumVert(); ind++) {
        LOG_DEBUG("tex %u %f %f", ind, m_mesh.getMeshFrontentData()->getTexCoord(ind).x, m_mesh.getMeshFrontentData()->getTexCoord(ind).y);
    }

    m_mesh.frontendBackendTransferInternal(m_loader);
}

void BitmapFont::readKerningPairs(illFileSystem::File * file, size_t size) {
    unsigned int numPairs = (unsigned int) size / 10;

    for(unsigned int pair = 0; pair < numPairs; pair++) {
        uint32_t firstChar;
        file->readL32(firstChar);
        assert(firstChar < NUM_CHARS);

        uint32_t secondChar;
        file->readL32(secondChar);
        assert(secondChar < NUM_CHARS);

        uint16_t amount;
        file->readL16(amount);
                
        m_kerningPairs[(char) firstChar][(char) secondChar] = (glm::mediump_float) (int16_t) amount;
    }
}

}
