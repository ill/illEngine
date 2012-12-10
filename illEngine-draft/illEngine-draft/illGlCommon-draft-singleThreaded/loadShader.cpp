#include <cstring>
#include <GL/glew.h>

#include "GlRenderer.h"
#include "../illGraphics-draft-singleThreaded/Material/Shader.h"

#include "../illUtil-draft-singleThreaded/physFsUtil.h" //TODO: temporary

#include "glLogging.h"
#include "../illGraphics-draft-singleThreaded/graphicsLogging.h"

namespace GlCommon {

void GlRenderer::loadShader(void ** shaderData, uint64_t featureMask) {
    /////////////////////////////////////
    //determine shader type and path

    std::string path;
    unsigned int shaderType;
    std::string defines;

    if(featureMask & Graphics::Shader::SHADER_3D_VERT) {
        shaderType = GL_VERTEX_SHADER;
        path = "shaders/main.vert";
    }

    if(featureMask & Graphics::Shader::SHADER_FORWARD_FRAG) {
        shaderType = GL_FRAGMENT_SHADER;
        path = "shaders/forward.frag";
    }

    ///////////////////////////////////////
    //determine shader includes
    if(featureMask & Graphics::Shader::SHADER_POSITIONS) {
        defines += "#define POSITION_TRANSFORM\n";
    }

    if(featureMask & Graphics::Shader::SHADER_NORMALS) {
        defines += "#define NORMAL_ATTRIBUTE\n";
    }

    if(featureMask & Graphics::Shader::SHADER_TEX_COORDS) {
        defines += "#define TEX_COORD_ATTRIBUTE\n";
    }

    if(featureMask & Graphics::Shader::SHADER_TANGENTS) {
        defines += "#define TANGENT_ATTRIBUTE\n";
    }

    if(featureMask & Graphics::Shader::SHADER_DIFFUSE_MAP) {
        defines += "#define DIFFUSE_MAP\n";
    }

    if(featureMask & Graphics::Shader::SHADER_SPECULAR_MAP) {
        defines += "#define SPECULAR_MAP\n";
    }

    if(featureMask & Graphics::Shader::SHADER_EMISSIVE_MAP) {
        defines += "#define EMISSIVE_MAP\n";
    }

    if(featureMask & Graphics::Shader::SHADER_NORMAL_MAP) {
        defines += "#define NORMAL_MAP\n";
    }

    loadShaderInternal(shaderData, path, shaderType, defines);
}

void GlRenderer::loadShaderInternal(void ** shaderData, const std::string& path, unsigned int shaderType, const std::string& defines) {    
    //////////////////////////////////
    //declare stuff
    GLint status; //status of shader
    const GLchar * programTextBuffer = NULL;
    GLint programTextBufferLength;

    programTextBufferLength = readFile(path, (char **) &programTextBuffer);

    ///////////////////////////////////////////
    //create the shader
    const GLchar * programTexts[2] = {defines.c_str(), programTextBuffer};
    const GLint programLengths[2] = {(GLint) defines.length(), (GLint) programTextBufferLength};

    GLuint shader = glCreateShader(shaderType);   
    glShaderSource(shader, 2, programTexts, programLengths);

    glCompileShader(shader);
    delete[] programTextBuffer;

    ///////////////////////////////////////////
    //print info log
#if ENABLE_LOG_DEBUG_GRAPHICS
    GLint infologLength = 0;
    GLint charsWritten = 0;
    GLchar *infoLog;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0) {
        infoLog = new GLchar[infologLength];

        if (infoLog == NULL) {
            LOG_FATAL_ERROR("Could not allocate Shader InfoLog buffer");
        }

        glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
        
        LOG_INFO("Shader Log: %s ", path.c_str());
        logger->printMessage(LogDestination::MT_INFO, infoLog);

        delete[] infoLog;
    }
#endif

    ///////////////////////////////////////////
    //check status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (!status) {
        LOG_FATAL_ERROR("Error compiling shader: %s", path.c_str());
    }

    ERROR_CHECK_OPENGL;

    *shaderData = new GLuint;
    memcpy(*shaderData, &shader, sizeof(GLuint));
}

void GlRenderer::unloadShader(void ** shaderData) {
    glDeleteShader(*(GLuint *)(*shaderData));
    delete (GLuint *) *shaderData;
    *shaderData = NULL;
}

}
