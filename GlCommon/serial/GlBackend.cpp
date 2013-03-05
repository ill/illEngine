#include <glm/gtc/type_ptr.hpp>

#include "GlCommon/serial/glUtil.h"

#include "Logging/logging.h"
#include "GlCommon/glLogging.h"

#include "GlCommon/serial/GlBackend.h"

#include "Graphics/serial/Camera/Camera.h"
#include "Graphics/serial/BitmapFont.h"

namespace GlCommon {

void GlBackend::initialize() {
    glewExperimental = true;      //TODO: remove this once GLEW stops being retarded after an update
    if(glewInit() != GLEW_OK) {
        LOG_FATAL_ERROR("Glew failed to initialize");
    }

    //glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    //initialize the GL state vars
    //glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *)&m_currentProgram);

    //load the temporary font shader
    /*{
        std::vector<RefCountPtr<illGraphics::Shader> > shaders;

        illGraphics::Shader * shader = new illGraphics::Shader();
        shader->loadInternal(this, "shaders/tempFont.vert", GL_VERTEX_SHADER, "");

        shaders.push_back(RefCountPtr<illGraphics::Shader>(shader));

        shader = new illGraphics::Shader();
        shader->loadInternal(this, "shaders/tempFont.frag", GL_FRAGMENT_SHADER, "");

        shaders.push_back(RefCountPtr<illGraphics::Shader>(shader));

        m_debugShaderLoader = new illGraphics::ShaderProgramLoader(this, NULL);
        m_fontShader.loadInternal(m_debugShaderLoader, shaders);
    }*/
}

void GlBackend::uninitialize() {
    //m_fontShader.unload();
    //delete m_debugShaderLoader;

    //TODO: figure out how to clean up GLEW
    
    //glewDes
}

void GlBackend::beginFrame() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GlBackend::endFrame() {
    //TODO: figure this out later, for now the SDL Window flips the buffer
}

//TODO: the font rendering code will need some considerable rewrites
//void GlRenderer::beginFontRender(const illGraphics::BitmapFont &font) {
//    GLint program = getProgram(m_fontShader);
//    glUseProgram(getProgram(m_fontShader));
//
//    //position
//    /*{
//        GLint pos = getProgramAttribLocation(m_currentShaderProgram, "position");
//        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, (GLsizei) font.getMesh().m_meshFrontendData->getVertexSize(), (char *)NULL + font.getMesh().m_meshFrontendData->getPositionOffset());
//        enableVertexAttribArray(pos);
//    }*/
//}
//
//void GlRenderer::renderCharacter(const illGraphics::Camera& camera, const glm::mat4& transform, const illGraphics::BitmapFont &font, unsigned char character) {
//    //TODO: assert that beginFontRender was called earlier and nothing else modified the state
//
//    //model view projection
//    /*{
//        GLint mvp = getProgramUniformLocation(m_currentShaderProgram, "modelViewProjectionMatrix");
//        glUniformMatrix4fv(mvp, 1, false, glm::value_ptr(camera.getCanonical() * transform));
//    }*/
//}

}
