#include <GL/glew.h>
#include "serial-illLogging/logging.h"
#include "glLogging.h"

#include "GlRenderer.h"

namespace GlCommon {

void GlRenderer::initialize() {
    glewExperimental = true;      //TODO: remove this once GLEW stops being retarded after an update
    if(glewInit() != GLEW_OK) {
        LOG_FATAL_ERROR("Glew failed to initialize");
    }
}

void GlRenderer::uninitialize() {
    //TODO: figure out how to clean up GLEW
    
    //glewDes
}

void GlRenderer::beginFrame() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GlRenderer::endFrame() {
    //TODO: figure this out later, for now the SDL Window flips the buffer
}

}