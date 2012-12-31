#include <cstring>

#include <IL/il.h>          //TODO: temporary for now, on PC this is fine, usually nice ways to load image files exist on ios and android
//TODO: support loading s3tc and other compressed textures depending on the device

#include <GL/glew.h>

#include "GlRenderer.h"
#include "Graphics/serial/Material/Texture.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"

namespace GlCommon {

void GlRenderer::loadTexture(void ** textureData, const illGraphics::TextureLoadArgs& loadArgs) {
    //////////////////////////////////
    //declare stuff
    char * textureMemBuffer;
    size_t textureMemBufferLength;
    ILuint ilTexture;
    ILubyte* imageData = NULL;
    int paddedWidth;
    int paddedHeight;

    ///////////////////////////////////////////
    //copy texture from archive to buffer    
    illFileSystem::File * openFile = illFileSystem::fileSystem->openRead(loadArgs.m_path.c_str());
    textureMemBufferLength = openFile->getSize();

    textureMemBuffer = new char[textureMemBufferLength];

    openFile->read(textureMemBuffer, textureMemBufferLength);

    delete openFile;

    /////////////////////////////////
    //load image with DevIL
    ilGenImages(1, &ilTexture);
    ilBindImage(ilTexture);

    //make origin of images be lower left corner at all times
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    ilEnable(IL_ORIGIN_SET);

    if (!ilLoadL(IL_TYPE_UNKNOWN, textureMemBuffer, (ILuint) textureMemBufferLength)) {
        LOG_FATAL_ERROR("Error loading image %s", loadArgs.m_path.c_str());
    }

    ///////////////////////////////////
    //load the texture into OpenGL
    if (!ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE)) {
        LOG_FATAL_ERROR("Error converting image %s to raw data", loadArgs.m_path.c_str());
    }

    int width = ilGetInteger(IL_IMAGE_WIDTH);
    int height = ilGetInteger(IL_IMAGE_HEIGHT);
    int imageBPP = ilGetInteger(IL_IMAGE_BPP);

    //////////////////////////////////
    //pad image if not power of 2

    //not necessary on newer hardware.  I'm not even supporting the kind of OpenGL that doesn't allow this

    /*paddedWidth = nextPow2(width);
    paddedHeight = nextPow2(height);

    if (paddedWidth != width || paddedHeight != height) {
    imageData = new ILubyte[paddedWidth * imageBPP * paddedHeight];

    gluScaleImage(ilGetInteger(IL_IMAGE_FORMAT), width, height, GL_UNSIGNED_BYTE, ilGetData(), paddedWidth,
    paddedHeight, GL_UNSIGNED_BYTE, imageData);
    } 
    else {*/
    imageData = ilGetData();
    //}

    /////////////////////////////
    //set texture attributes
    GLuint texture;

    glGenTextures(1, &texture);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, loadArgs.m_wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, loadArgs.m_wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    /////////////////////////////
    //create texture in OpenGL
    gluBuild2DMipmaps(GL_TEXTURE_2D, imageBPP, /*paddedWidth, paddedHeight,*/width, height, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, imageData);

    /////////////////////////////
    //clean up image data
    /*if (paddedWidth != width || paddedHeight != height) {
    delete[] imageData;
    }*/

    ilDeleteImages(1, &ilTexture);

    ///////////////////////////////////
    //clean up
    delete[] textureMemBuffer;

    *textureData = new GLuint;
    memcpy(*textureData, &texture, sizeof(GLuint));
}

void GlRenderer::unloadTexture(void ** textureData) {
    glDeleteTextures(1, (GLuint *)(*textureData));
    delete (GLuint *) *textureData;
    *textureData = NULL;
}

}
