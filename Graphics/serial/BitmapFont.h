#ifndef ILL_BITMAP_FONT_H_
#define ILL_BITMAP_FONT_H_

#include <map>
#include <vector>
#include <string>
#include "Graphics/serial/Material/Texture.h"
#include "Graphics/serial/Model/Mesh.h"

namespace illFileSystem {
    class File;
}

namespace illGraphics {

const unsigned int NUM_CHARS = 256;

class GraphicsBackend;

struct BitmapFontLoadArgs {
    std::string m_path;
};

/**
Font is still in development.  At the moment I'm using angel code bitmap font descriptors but I plan on
supporting loading actual font files using the freetype library in a bit.

I also plan on trying out this article sometime for vector graphics. http://http.developer.nvidia.com/GPUGems3/gpugems3_ch25.html

This will most likely get redesigned later so there's some common font rendering code and different ways to load fonts in.
*/
class BitmapFont : public ResourceBase<BitmapFont, BitmapFontLoadArgs, GraphicsBackend> {
public:
    struct CharData {
        uint16_t m_meshIndex;               ///<the index of the first vertex in the mesh that contains geometry data for this character
        glm::mediump_float m_advance;       ///<how much to advance the write location when this character is written
        uint8_t m_texturePage;              ///<which texture is this character in
    };

    BitmapFont()
        : ResourceBase()
    {}

    virtual ~BitmapFont() {
        unload();
    }

    virtual void unload();
    virtual void reload(GraphicsBackend * backend);
    
    /**
    Gets the width of a line of text in the buffer provided.
    Stops at the next newline and sets the text pointer to point to the character after the newline.

    This is unscaled so if rendering a scaled font multiply this by the horizontal scale.
    */
    glm::mediump_float getLineWidth(char const ** text) const;

    /**
    Gets the rectangular dimensions that would be taken up by some text.

    This is unscaled so if rendering a scaled font multiply this by the horizontal and vertical scale
    */
    glm::vec2 getPrintDimensions(const char * text) const;

    /**
    Gets the color code from a block of text.
    First checks if this is in fact a color code and returns true.  If not a color code, nothing happens and returns false.

    Sets the text pointer to point to the character after the color code.

    Color codes can start with a carat and a number 0 through 9.  This matches the Quake 3 color codes. http://www.computerhope.com/issues/ch000658.htm
    ^0 : black
    ^1 : red
    ^2 : green
    ^3 : yellow
    ^4 : blue
    ^5 : light blue
    ^6 : pink
    ^7 : white

    //these don't come from Quake 3, I added them myself
    ^8 : purple
    ^9 : gray

    Color codes can also start with the pound character and followed by either an RGB or RGBA hex color.
    #FFFFFF would be white with alpha of 1 for example
    #00000022 would be barely visible black

    The color code determines what color the following text after the color code will be.  This can let users create colorful player names :D
    Also it's useful for coloring your own text.

    @return Whether or not that was a color code.
    */
    static bool getColorCode(const char ** text, glm::vec4& destination); 

    inline glm::mediump_float getPaddingUp() const {
        return m_paddingUp;
    }

    inline glm::mediump_float getPaddingDown() const {
        return m_paddingDown;
    }

    inline glm::mediump_float getPaddingLeft() const {
        return m_paddingLeft;
    }

    inline glm::mediump_float getPaddingRight() const {
        return m_paddingRight;
    }

    inline glm::mediump_float getSpacingHorz() const {
        return m_spacingHorz;
    }

    inline glm::mediump_float getSpacingVert() const {
        return m_spacingVert;
    }

    inline glm::mediump_float getLineHeight() const {
        return m_lineHeight;
    }

    inline glm::mediump_float getLineBase() const {
        return m_lineBase;
    }

    inline glm::mediump_float getKerning(unsigned char first, unsigned char second) const {
        std::map<unsigned char, std::map<unsigned char, glm::mediump_float> >::const_iterator firstIter = m_kerningPairs.find(first);

        if(firstIter != m_kerningPairs.end()) {
            std::map<unsigned char, glm::mediump_float>::const_iterator secondIter = firstIter->second.find(second);

            if(secondIter != firstIter->second.end()) {
                return secondIter->second;
            }
        }

        return 0.0f;
    }

    inline const CharData& getCharData(unsigned char character) const {
        return m_charData[character];
    }

    inline const Mesh& getMesh() const {
        return m_mesh;
    }

    inline const Texture& getPageTexture(unsigned int page) const {
        return m_pageTextures[page];
    }
    
private:
    void readInfo(illFileSystem::File * file, size_t size);
    void readCommon(illFileSystem::File * file, unsigned int& textureWidth, unsigned int& textureHeight);
    void readPages(illFileSystem::File * file, size_t size);
    void readChars(illFileSystem::File * file, size_t size, unsigned int textureWidth, unsigned int textureHeight);
    void readKerningPairs(illFileSystem::File * file, size_t size);
    
    glm::mediump_float m_paddingUp;
    glm::mediump_float m_paddingDown;
    glm::mediump_float m_paddingLeft;
    glm::mediump_float m_paddingRight;

    glm::mediump_float m_spacingHorz;
    glm::mediump_float m_spacingVert;

    std::vector<Texture> m_pageTextures;
    Mesh m_mesh;

    glm::mediump_float m_lineHeight;
    glm::mediump_float m_lineBase;

    CharData m_charData[NUM_CHARS];
    std::map<unsigned char, std::map<unsigned char, glm::mediump_float> > m_kerningPairs;      ///<Kerning pairs, look up by first character, then by second character, than by amount    
};

}

#endif