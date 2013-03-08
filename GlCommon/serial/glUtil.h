#ifndef ILL_GL_UTIL_H_
#define ILL_GL_UTIL_H_

#include <GL/glew.h>
#include "Logging/logging.h"
#include "Graphics/serial/Material/Texture.h"
#include "Graphics/serial/Material/ShaderProgram.h"

inline GLuint getProgram(const illGraphics::ShaderProgram& program) {
    return *((GLuint *) program.getShaderProgram());
}

inline GLuint getTexture(const illGraphics::Texture& texture) {
    return *((GLuint *) texture.getTextureData());
}

inline GLint getProgramUniformLocation(GLuint program, const char * name) {
    GLint loc = glGetUniformLocation(program, name);

    if(loc == -1) {
        LOG_FATAL_ERROR("Unknown uniform %s", name);
    }

    return loc;
}

inline GLint getProgramAttribLocation(GLuint program, const char * name) {
    GLint loc = glGetAttribLocation(program, name);

    if(loc == -1) {
        LOG_FATAL_ERROR("Unknown attribute %s", name);
    }

    return loc;
}

#endif