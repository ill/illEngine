#include <glm/gtc/type_ptr.hpp>

#include "DeferredShadingBackendGl3_3.h"
#include "glCommon/glLogging.h"
#include "glCommon/serial/glUtil.h"
#include "Graphics/serial/Camera/Camera.h"

#include "RendererCommon/serial/StaticMeshNode.h"

namespace illDeferredShadingRenderer {

void DeferredShadingBackendGl3_3::initialize(const glm::uvec2 screenResolution) {
    uninitialize();

    glGenFramebuffers(1, &m_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);

    glActiveTexture(GL_TEXTURE0);

    glGenTextures(REN_LAST, m_renderTextures);

    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_DEPTH]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screenResolution.x, screenResolution.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);

    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_NORMAL]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB10_A2, screenResolution.x, screenResolution.y, 0, GL_RGBA, GL_UNSIGNED_INT_10_10_10_2, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);   

    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_DIFFUSE]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenResolution.x, screenResolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_SPECULAR]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenResolution.x, screenResolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_DIFFUSE_ACCUMULATION]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenResolution.x, screenResolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_SPECULAR_ACCUMULATION]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenResolution.x, screenResolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_renderTextures[REN_DEPTH], 0);

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    switch (status) {
    case GL_FRAMEBUFFER_UNSUPPORTED:
        LOG_FATAL_ERROR("Frame Buffers unsupported.  Bummer.");
        break;
    case GL_FRAMEBUFFER_UNDEFINED:
        LOG_FATAL_ERROR("Frame Buffers undefined.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        LOG_FATAL_ERROR("Incomplete layer targets!");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        LOG_FATAL_ERROR("Incomplete attachment");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        LOG_FATAL_ERROR("Missing attachment");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        LOG_FATAL_ERROR("Incomplete draw buffer");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        LOG_FATAL_ERROR("Incomplete read buffer");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        LOG_FATAL_ERROR("Incomplete multisample");
        break;
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        LOG_DEBUG("Frame Buffer good to go");
        break;
    default:
        LOG_FATAL_ERROR("Unknown frame buffer error!");
        break;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ERROR_CHECK_OPENGL;

    m_state = State::INITIALIZED;
}

void DeferredShadingBackendGl3_3::uninitialize() {
    if(m_state != State::INITIALIZED) {
        return;
    }

    glDeleteFramebuffers(1, &m_gBuffer);
    glDeleteTextures(REN_LAST, m_renderTextures);

    m_deferredPointLightProgram.unload();
    m_deferredSpotLightProgram.unload();

    m_state = State::INITIALIZED;
}

void DeferredShadingBackendGl3_3::setupFrame() {
    //clear the render target datas
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
    
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTextures[REN_NORMAL], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_renderTextures[REN_DIFFUSE], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_renderTextures[REN_SPECULAR], 0);

    GLenum mrt[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, mrt);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DeferredShadingBackendGl3_3::setupViewport(const illGraphics::Camera& camera) {
    glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y, camera.getViewportDimensions().x, camera.getViewportDimensions().y);

    //TODO, make this function take an argument for the viewport, and also set up the backend to be doing stuff to do with the viewport and occlusion queries
}

void DeferredShadingBackendGl3_3::retreiveCellQueries(size_t viewPort) {
    //TODO: this doesn't work yet
}

void DeferredShadingBackendGl3_3::depthPass(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera) {
    //disable blend
    glDisable(GL_BLEND);

    //disable color mask
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    //enable depth func less
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
   
    //enable backface culling
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    //the unanimated solid meshes
    for(auto shaderIter = renderQueues.m_depthPassSolidStaticMeshes.begin(); shaderIter != renderQueues.m_depthPassSolidStaticMeshes.end(); shaderIter++) {
        const illGraphics::ShaderProgram * program = shaderIter->first;
        auto& meshes = shaderIter->second;

        GLuint prog = getProgram(*program);
        glUseProgram(prog);

        GLint posAttrib = getProgramAttribLocation(prog, "positionIn");
        glEnableVertexAttribArray(posAttrib);

        for(auto materialIter = meshes.begin(); materialIter != meshes.end(); materialIter++) {
            const illGraphics::Material * material = materialIter->first;
            auto meshes = materialIter->second;

            //TODO: material specific states
            //TODO: skinning attrib

            for(auto meshIter = meshes.begin(); meshIter != meshes.end(); meshIter++) {
                const illGraphics::Mesh * mesh = meshIter->first;
                auto meshNodes = meshIter->second;

                for(auto nodeIter = meshNodes.begin(); nodeIter !=  meshNodes.end(); nodeIter++) {
                    const illRendererCommon::StaticMeshNode * node = *nodeIter;

                    glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 1, false, glm::value_ptr(camera.getModelViewProjection() * node->getTransform()));

                    //bind VBO
                    {
                        GLuint buffer = *((GLuint *) mesh->getMeshBackendData() + 0);
                        glBindBuffer(GL_ARRAY_BUFFER, buffer);
                    }

                    //setup positions
                    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, (GLsizei) mesh->getMeshFrontentData()->getVertexSize(), (char *)NULL + mesh->getMeshFrontentData()->getPositionOffset());                

                    //TODO: skinning

                    //bind IBO
                    {
                        GLuint buffer = *((GLuint *) mesh->getMeshBackendData() + 1);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
                    }

                    //DRAW!!!!
                    glDrawRangeElements(GL_TRIANGLES, 0, mesh->getMeshFrontentData()->getNumTri() * 3, mesh->getMeshFrontentData()->getNumTri() * 3, GL_UNSIGNED_SHORT, (char *)NULL);
                }
            }

            //TODO: disable skinning attrib
        }

        glDisableVertexAttribArray(posAttrib);
    }

    renderQueues.m_depthPassSolidStaticMeshes.clear();
}

void renderDebugTexture(GLuint texture) {
    //debug
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0.0f, 1.0f,
        0.0f, 1.0f,
        -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glColor3f(1.0f, 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE1);
    glDisable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE2);
    glDisable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE3);
    glDisable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, 1.0f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, 0.0f);
    glEnd();
}

void DeferredShadingBackendGl3_3::render(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera) {
    //disable depth mask
    glDepthMask(GL_FALSE);

    //enable color mask
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    //enable depth func equal
    glDepthFunc(GL_EQUAL);

    //the unanimated solid meshes
    for(auto shaderIter = renderQueues.m_solidStaticMeshes.begin(); shaderIter != renderQueues.m_solidStaticMeshes.end(); shaderIter++) {
        const illGraphics::ShaderProgram * program = shaderIter->first;
        auto& meshes = shaderIter->second;

        GLuint prog = getProgram(*program);
        glUseProgram(prog);

        GLint posAttrib = getProgramAttribLocation(prog, "positionIn");
        glEnableVertexAttribArray(posAttrib);

        //TODO: forward rendering on fullbright solid objects
        GLint normAttrib = getProgramAttribLocation(prog, "normalIn");
        glEnableVertexAttribArray(normAttrib);

        for(auto materialIter = meshes.begin(); materialIter != meshes.end(); materialIter++) {
            const illGraphics::Material * material = materialIter->first;
            auto meshes = materialIter->second;

            //pass material colors
            glUniform3fv(getProgramUniformLocation(prog, "diffuseColor"), 1, glm::value_ptr(material->getLoadArgs().m_diffuseBlend));
            glUniform4fv(getProgramUniformLocation(prog, "specularColor"), 1, glm::value_ptr(material->getLoadArgs().m_specularBlend));

            //textures

            //-2 means unneeded, -1 means needed and will be initialized afterwards
            GLint texCoordAttrib = -2;
            GLint tangentsAttrib = -2;
            GLint bitangentsAttrib;     //bitangents implicitly needed along with tangents

            if(material->getLoadArgs().m_diffuseTextureIndex >= 0) {
                texCoordAttrib = -1;

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, getTexture(*material->getDiffuseTexture()));
                glUniform1i(getProgramUniformLocation(prog, "diffuseMap"), 0);
            }

            if(material->getLoadArgs().m_specularTextureIndex >= 0) {
                texCoordAttrib = -1;

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, getTexture(*material->getSpecularTexture()));
                glUniform1i(getProgramUniformLocation(prog, "specularMap"), 1);
            }

            if(material->getLoadArgs().m_normalTextureIndex >= 0) {
                texCoordAttrib = -1;
                tangentsAttrib = -1;

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, getTexture(*material->getNormalTexture()));
                glUniform1i(getProgramUniformLocation(prog, "normalMap"), 2);
            }

            if(texCoordAttrib == -1) {
                texCoordAttrib = getProgramAttribLocation(prog, "texCoordIn");
                glEnableVertexAttribArray(texCoordAttrib);
            }

            if(tangentsAttrib == -1) {
                tangentsAttrib = getProgramAttribLocation(prog, "tangentIn");
                glEnableVertexAttribArray(tangentsAttrib);

                bitangentsAttrib = getProgramAttribLocation(prog, "bitangentIn");
                glEnableVertexAttribArray(bitangentsAttrib);
            }

            //TODO: skinning attrib

            for(auto meshIter = meshes.begin(); meshIter != meshes.end(); meshIter++) {
                const illGraphics::Mesh * mesh = meshIter->first;
                auto meshNodes = meshIter->second;

                for(auto nodeIter = meshNodes.begin(); nodeIter !=  meshNodes.end(); nodeIter++) {
                    const illRendererCommon::RenderQueues::StaticMeshInfo& meshInfo = *nodeIter;

                    glm::mat4 modelViewProjection = camera.getModelViewProjection() * meshInfo.m_node->getTransform();

                    glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 
                        1, false, glm::value_ptr(modelViewProjection));

                    glUniformMatrix3fv(getProgramUniformLocation(prog, "normalMat"), 
                        1, false, glm::value_ptr(glm::mat3(modelViewProjection)));

                    //bind VBO
                    {
                        GLuint buffer = *((GLuint *) mesh->getMeshBackendData() + 0);
                        glBindBuffer(GL_ARRAY_BUFFER, buffer);
                    }

                    //bind IBO
                    {
                        GLuint buffer = *((GLuint *) mesh->getMeshBackendData() + 1);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
                    }

                    //setup positions
                    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 
                        (GLsizei) mesh->getMeshFrontentData()->getVertexSize(), (char *)NULL + mesh->getMeshFrontentData()->getPositionOffset());

                    //setup tex coords
                    glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 
                        (GLsizei) mesh->getMeshFrontentData()->getVertexSize(), (char *)NULL + mesh->getMeshFrontentData()->getTexCoordOffset());

                    //setup normals
                    glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 
                        (GLsizei) mesh->getMeshFrontentData()->getVertexSize(), (char *)NULL + mesh->getMeshFrontentData()->getNormalOffset());
                    
                    //setup tangents
                    glVertexAttribPointer(tangentsAttrib, 3, GL_FLOAT, GL_FALSE, 
                        (GLsizei) mesh->getMeshFrontentData()->getVertexSize(), (char *)NULL + mesh->getMeshFrontentData()->getTangentOffset());

                    //setup bitangents
                    glVertexAttribPointer(bitangentsAttrib, 3, GL_FLOAT, GL_FALSE, 
                        (GLsizei) mesh->getMeshFrontentData()->getVertexSize(), (char *)NULL + mesh->getMeshFrontentData()->getBitangentOffset());

                    //TODO: skinning

                    //DRAW!!!!
                    glDrawRangeElements(GL_TRIANGLES, 0, 
                        mesh->getMeshFrontentData()->getNumTri() * 3, mesh->getMeshFrontentData()->getNumTri() * 3, GL_UNSIGNED_SHORT, (char *)NULL);
                }
            }

            if(texCoordAttrib >= 0) {
                glDisableVertexAttribArray(texCoordAttrib);
            }

            if(tangentsAttrib >= 0) {
                glDisableVertexAttribArray(tangentsAttrib);
                glDisableVertexAttribArray(bitangentsAttrib);
            }

            //TODO: disable skinning attrib
        }

        glDisableVertexAttribArray(posAttrib);
        glDisableVertexAttribArray(normAttrib);
    }

    if(m_debugMode == DebugMode::NONE || m_debugMode == DebugMode::LIGHT_POS || m_debugMode == DebugMode::DIFFUSE_ACCUMULATION || m_debugMode == DebugMode::SPECULAR_ACCUMULATION) {
    }
    else {
        switch(m_debugMode) {
        case DebugMode::DEPTH:
            renderDebugTexture(m_renderTextures[REN_DEPTH]);
            break;

        case DebugMode::NORMAL:
            renderDebugTexture(m_renderTextures[REN_NORMAL]);
            break;

        case DebugMode::DIFFUSE:
            renderDebugTexture(m_renderTextures[REN_DIFFUSE]);
            break;

        case DebugMode::SPECULAR:
            renderDebugTexture(m_renderTextures[REN_SPECULAR]);
            break;
        }
    }
}

}