#ifndef __GL_LOGGING_H__
#define __GL_LOGGING_H__

#include <GL/glew.h>

#include "serial-illLogging/logging.h"

#ifndef NDEBUG      //Debug Build

/*
Checks for OpenGL errors and prints them out if there are any
*/
#define ERROR_CHECK_OPENGL do{\
    GLenum glErr = glGetError();\
    bool errorOccured = false;\
    while (glErr != GL_NO_ERROR) {\
        errorOccured = true;\
        LOG_ERROR("OpenGL Error: %s", gluErrorString(glErr));\
        glErr = glGetError();\
    }\
    if(errorOccured) LOG_FATAL_ERROR("OpenGL Errors Occured.");\
} while(0)

#else               //Release Build

#define ERROR_CHECK_OPENGL do {} while(0)

#endif

#endif