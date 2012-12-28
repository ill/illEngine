#include <GL/glew.h>

#include "GlRenderer.h"
#include "serial-illGraphics/Material/Shader.h"

#include "glLogging.h"
#include "serial-illGraphics/graphicsLogging.h"

namespace GlCommon {

void GlRenderer::loadShaderProgram(void ** programData, const std::vector<RefCountPtr<Graphics::Shader> >& shaderList) {
    //////////////////////////////////
    //declare stuff
    GLint status; //status of shader

    ///////////////////////////////////////////
    //create the shader program
    GLuint program = glCreateProgram(); 

    for(std::vector<RefCountPtr<Graphics::Shader> >::const_iterator iter = shaderList.begin(); iter != shaderList.end(); iter++) {
        glAttachShader(program, *(GLuint*) ((*iter)->getShaderData()));
    }

    glLinkProgram(program);

    ///////////////////////////////////////////
    //print info log
#if ENABLE_LOG_DEBUG_GRAPHICS
    GLint infologLength = 0;
    GLint charsWritten = 0;
    GLchar *infoLog;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0) {
        infoLog = new GLchar[infologLength];

        if (infoLog == NULL) {
            LOG_FATAL_ERROR("Could not allocate Shader Program InfoLog buffer");
        }

        glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);

        LOG_INFO("ShaderProgram Log:");
        illLogging::logger->printMessage(illLogging::LogDestination::MT_INFO, infoLog);

        delete[] infoLog;
    }
#endif

    ///////////////////////////////////////////
    //check status
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (!status) {
        LOG_FATAL_ERROR("Error linking shader program");
    }

    ERROR_CHECK_OPENGL;

    *programData = new GLuint;
    memcpy(*programData, &program, sizeof(GLuint));
}

void GlRenderer::unloadShaderProgram(void ** programData) {
    glDeleteProgram(*(GLuint *)(*programData));
    delete (GLuint *) *programData;
    *programData = NULL;
}

}