#include <glm/gtc/type_ptr.hpp>

#include "DeferredShadingBackendGl3_3.h"
#include "glCommon/glLogging.h"
#include "glCommon/serial/glUtil.h"
#include "Graphics/serial/Camera/Camera.h"

#include "RendererCommon/serial/StaticMeshNode.h"
#include "RendererCommon/serial/LightNode.h"

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

    //load the light shaders
    m_internalShaderProgramLoader = new illGraphics::ShaderProgramLoader(m_graphicsBackend, NULL);

    {
        RefCountPtr<illGraphics::Shader> lightVertexShader(new illGraphics::Shader());
        lightVertexShader->loadInternal(m_graphicsBackend, "shaders/deferredPhongLighting.vert", GL_VERTEX_SHADER, "");

        //point light
        {
            std::vector<RefCountPtr<illGraphics::Shader> > shaders;
            shaders.push_back(lightVertexShader);

            illGraphics::Shader * fragShader = new illGraphics::Shader();
            fragShader->loadInternal(m_graphicsBackend, "shaders/deferredPhongLighting.frag", GL_FRAGMENT_SHADER, "#define POINT_LIGHT");

            shaders.push_back(RefCountPtr<illGraphics::Shader>(fragShader));
            m_deferredPointLightProgram.loadInternal(m_internalShaderProgramLoader, shaders);
        }

        //spot light
        {
            std::vector<RefCountPtr<illGraphics::Shader> > shaders;
            shaders.push_back(lightVertexShader);

            illGraphics::Shader * fragShader = new illGraphics::Shader();
            fragShader->loadInternal(m_graphicsBackend, "shaders/deferredPhongLighting.frag", GL_FRAGMENT_SHADER, "#define SPOT_LIGHT");

            shaders.push_back(RefCountPtr<illGraphics::Shader>(fragShader));
            m_deferredSpotLightProgram.loadInternal(m_internalShaderProgramLoader, shaders);
        }

        //direction light
        /*{
            std::vector<RefCountPtr<illGraphics::Shader> > shaders;
            shaders.push_back(lightVertexShader);

            illGraphics::Shader * fragShader = new illGraphics::Shader();
            fragShader->loadInternal(m_graphicsBackend, "shaders/deferredPhongLighting.frag", GL_FRAGMENT_SHADER, "#define DIRECTIONAL_LIGHT");

            shaders.push_back(RefCountPtr<illGraphics::Shader>(fragShader));
            m_deferredDirectionLightProgram.loadInternal(m_internalShaderProgramLoader, shaders);
        }*/
    }

    //load the light volumes
    //TODO: for now just rendering a giant box using immediate mode

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

    delete m_internalShaderProgramLoader;

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
                    if(m_debugOcclusion) {
                        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y,
                            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);

                        glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 1, false, glm::value_ptr(m_occlusionCamera->getModelViewProjection() * node->getTransform()));

                        glDrawRangeElements(GL_TRIANGLES, 0, mesh->getMeshFrontentData()->getNumTri() * 3, mesh->getMeshFrontentData()->getNumTri() * 3, GL_UNSIGNED_SHORT, (char *)NULL);

                        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y + camera.getViewportDimensions().y / 2,
                            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
                    }

                    glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 1, false, glm::value_ptr(camera.getModelViewProjection() * node->getTransform()));

                    glDrawRangeElements(GL_TRIANGLES, 0, mesh->getMeshFrontentData()->getNumTri() * 3, mesh->getMeshFrontentData()->getNumTri() * 3, GL_UNSIGNED_SHORT, (char *)NULL);
                }
            }

            //TODO: disable skinning attrib
        }

        glDisableVertexAttribArray(posAttrib);
    }

    renderQueues.m_depthPassSolidStaticMeshes.clear();

    if(m_debugOcclusion) {
        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y, camera.getViewportDimensions().x, camera.getViewportDimensions().y);
    }
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

void DeferredShadingBackendGl3_3::renderGbuffer(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera) {
    //the unanimated solid meshes
    for(auto shaderIter = renderQueues.m_solidStaticMeshes.begin(); shaderIter != renderQueues.m_solidStaticMeshes.end(); shaderIter++) {
        const illGraphics::ShaderProgram * program = shaderIter->first;
        auto& materials = shaderIter->second;

        GLuint prog = getProgram(*program);
        glUseProgram(prog);

        GLint posAttrib = getProgramAttribLocation(prog, "positionIn");
        glEnableVertexAttribArray(posAttrib);

        //TODO: forward rendering on fullbright solid objects
        GLint normAttrib = getProgramAttribLocation(prog, "normalIn");
        glEnableVertexAttribArray(normAttrib);

        for(auto materialIter = materials.begin(); materialIter != materials.end(); materialIter++) {
            const illGraphics::Material * material = materialIter->first;
            auto& meshes = materialIter->second;

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
                auto& meshNodes = meshIter->second;

                for(auto nodeIter = meshNodes.begin(); nodeIter !=  meshNodes.end(); nodeIter++) {
                    const illRendererCommon::RenderQueues::StaticMeshInfo& meshInfo = *nodeIter;
                    
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
                    if(m_debugOcclusion) {
                        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y,
                            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);

                        glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 
                            1, false, glm::value_ptr(m_occlusionCamera->getModelViewProjection() * meshInfo.m_node->getTransform()));

                        glUniformMatrix3fv(getProgramUniformLocation(prog, "normalMat"), 
                            1, false, glm::value_ptr(glm::mat3(m_occlusionCamera->getModelView() * meshInfo.m_node->getTransform())));

                        glDrawRangeElements(GL_TRIANGLES, 0, 
                            mesh->getMeshFrontentData()->getNumTri() * 3, mesh->getMeshFrontentData()->getNumTri() * 3, GL_UNSIGNED_SHORT, (char *)NULL);
                        
                        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y + camera.getViewportDimensions().y / 2,
                            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
                    }

                    glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 
                            1, false, glm::value_ptr(camera.getModelViewProjection() * meshInfo.m_node->getTransform()));

                        glUniformMatrix3fv(getProgramUniformLocation(prog, "normalMat"), 
                            1, false, glm::value_ptr(glm::mat3(camera.getModelView() * meshInfo.m_node->getTransform())));

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

    if(m_debugOcclusion) {
        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y, camera.getViewportDimensions().x, camera.getViewportDimensions().y);
    }
}

void DeferredShadingBackendGl3_3::renderAmbientPass(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera) {
    /*glUseProgram(m_ambientPassProgram->getShaderProgram());
   
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_DIFFUSE]);

    glUniform1i(m_ambientPassProgram->getUniformLocation("diffuseBuffer"), 0);
    glUniform3f(m_ambientPassProgram->getUniformLocation("ambientColor"), 0.1f, 0.1f, 0.1f);

    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(1.0f, 0.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(1.0f, 1.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, 1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, 0.0f);
    glEnd();*/
}

void DeferredShadingBackendGl3_3::renderEmissivePass(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera) {
}

void DeferredShadingBackendGl3_3::renderLights(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera) {
    //figure out the normalized planes sent to the shader for retreiving position from depth    
    float planes[2] = {
        camera.getFarVal() / (camera.getFarVal() - camera.getNearVal()),
        (camera.getFarVal() * camera.getNearVal()) / (camera.getFarVal() - camera.getNearVal()) //normally this is negated in a left handed coordinate system
    };

    //set the g buffer textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_DEPTH]);            //TODO: Optimize this...

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_NORMAL]);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_DIFFUSE]);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_SPECULAR]);

    for(auto lightTypeIter = renderQueues.m_lights.begin(); lightTypeIter != renderQueues.m_lights.end(); lightTypeIter++) {
        illGraphics::LightBase::Type lightType = lightTypeIter->first;
        auto& lights = lightTypeIter->second;
        
        GLuint prog;
        illGraphics::Mesh * lightVolume;

        switch(lightType) {
        case illGraphics::LightBase::Type::POINT:
            prog = getProgram(m_deferredPointLightProgram);
            lightVolume = &m_pointLightVolume;
            break;

        case illGraphics::LightBase::Type::SPOT:
            prog = getProgram(m_deferredSpotLightProgram);
            lightVolume = &m_spotLightVolume;
            break;

        /*case illGraphics::LightBase::Type::DIRECTIONAL: TODO
            prog = getProgram(m_deferredDirectionalLightProgram);
            lightVolume = &m_quad;
            break;*/
        }

        glUseProgram(prog);

        glUniform2fv(getProgramUniformLocation(prog, "planes"), 1, planes);
        
        glUniform1i(getProgramUniformLocation(prog, "depthBuffer"), 0);
        glUniform1i(getProgramUniformLocation(prog, "normalBuffer"), 1);
        glUniform1i(getProgramUniformLocation(prog, "diffuseBuffer"), 2);
        glUniform1i(getProgramUniformLocation(prog, "specularBuffer"), 3);

        for(auto lightIter = lights.begin(); lightIter != lights.end(); lightIter++) {
            illGraphics::LightBase * light = lightIter->first;
            auto& lightNodes = lightIter->second;

            glUniform1f(getProgramUniformLocation(prog, "intensity"), light->m_intensity);
            glUniform3fv(getProgramUniformLocation(prog, "lightColor"), 1, glm::value_ptr(light->m_color));

            glm::vec3 volumeScale;

            switch(lightType) {
            case illGraphics::LightBase::Type::POINT:
                glUniform1f(getProgramUniformLocation(prog, "attenuationStart"), 
                    static_cast<illGraphics::PointLight*>(light)->m_attenuationStart);

                glUniform1f(getProgramUniformLocation(prog, "attenuationEnd"), 
                    static_cast<illGraphics::PointLight*>(light)->m_attenuationEnd);

                volumeScale = glm::vec3(static_cast<illGraphics::PointLight*>(light)->m_attenuationEnd);

                break;

            case illGraphics::LightBase::Type::SPOT:
                glUniform1f(getProgramUniformLocation(prog, "attenuationStart"), 
                    static_cast<illGraphics::SpotLight*>(light)->m_attenuationStart);

                glUniform1f(getProgramUniformLocation(prog, "attenuationEnd"), 
                    static_cast<illGraphics::SpotLight*>(light)->m_attenuationEnd);

                glUniform1f(getProgramUniformLocation(prog, "coneStart"), 
                    static_cast<illGraphics::SpotLight*>(light)->m_coneStart);

                glUniform1f(getProgramUniformLocation(prog, "coneEnd"), 
                    static_cast<illGraphics::SpotLight*>(light)->m_coneEnd);

                volumeScale = glm::vec3(static_cast<illGraphics::SpotLight*>(light)->m_attenuationEnd);

                break;

            /*case illGraphics::LightBase::Type::DIRECTIONAL: TODO
                break;*/
            }

            for(auto nodeIter = lightNodes.begin(); nodeIter != lightNodes.end(); nodeIter++) {
                const illRendererCommon::LightNode * node = *nodeIter;
                
                if(m_debugOcclusion) {
                    planes[0] = m_occlusionCamera->getFarVal() / (m_occlusionCamera->getFarVal() - m_occlusionCamera->getNearVal());
                    planes[1] = (m_occlusionCamera->getFarVal() * m_occlusionCamera->getNearVal()) / (m_occlusionCamera->getFarVal() - m_occlusionCamera->getNearVal()); //normally this is negated in a left handed coordinate system

                    glUniform2fv(getProgramUniformLocation(prog, "planes"), 1, planes);

                    glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y,
                        camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);

                    glm::mat4 modelView = glm::scale(m_occlusionCamera->getModelView() * node->getTransform(), volumeScale);
                
                    glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 
                        1, false, glm::value_ptr(glm::scale(m_occlusionCamera->getModelViewProjection() * node->getTransform(), volumeScale)));
                
                    glUniformMatrix4fv(getProgramUniformLocation(prog, "modelView"), 
                        1, false, glm::value_ptr(modelView));
                
                    switch(lightType) {
                    case illGraphics::LightBase::Type::POINT:
                    case illGraphics::LightBase::Type::SPOT:
                        glUniform3fv(getProgramUniformLocation(prog, "lightPosition"), 
                            1, glm::value_ptr(getTransformPosition(m_occlusionCamera->getModelView() * node->getTransform())));

                        /*
                        if light volume intersects far plane, back face culling, otherwise front face culling
                        assuming the light center and attenuation end as sphere radius will suffice as a good test, 
                        unless your light is HUGE or the draw distance is TINY
                        in that case, you're doing it wrong!!!!
                        */
                        if(m_occlusionCamera->getViewFrustum().m_far.distance(getTransformPosition(node->getTransform())) 
                                < static_cast<illGraphics::PointLight*>(light)->m_attenuationEnd) {
                            glCullFace(GL_BACK);
                        }
                        else {
                            glCullFace(GL_FRONT);
                        }

                        break;

                    case illGraphics::LightBase::Type::DIRECTIONAL:
                        //TODO:
                        break;
                    }

                    //TODO: clearly I don't plan on using a box drawn in immediate mode for long, I will draw the light volume shapes soon with PROPER VBOs
                
                    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                    glBegin(GL_QUADS);
                    glVertex3f(1.0f, -1.0f, 1.0f);
                    glVertex3f(1.0f, 1.0f, 1.0f);
                    glVertex3f(-1.0f, 1.0f, 1.0f);
                    glVertex3f(-1.0f, -1.0f, 1.0f);

                    glVertex3f(-1.0f, -1.0f, -1.0f);
                    glVertex3f(-1.0f, 1.0f, -1.0f);
                    glVertex3f(1.0f, 1.0f, -1.0f);
                    glVertex3f(1.0f, -1.0f, -1.0f);

                    glVertex3f(-1.0f, -1.0f, 1.0f);
                    glVertex3f(-1.0f, 1.0f, 1.0f);
                    glVertex3f(-1.0f, 1.0f, -1.0f);
                    glVertex3f(-1.0f, -1.0f, -1.0f);

                    glVertex3f(1.0f, -1.0f, -1.0f);
                    glVertex3f(1.0f, 1.0f, -1.0f);
                    glVertex3f(1.0f, 1.0f, 1.0f);
                    glVertex3f(1.0f, -1.0f, 1.0f);

                    glVertex3f(1.0f, 1.0f, 1.0f);
                    glVertex3f(1.0f, 1.0f, -1.0f);
                    glVertex3f(-1.0f, 1.0f, -1.0f);
                    glVertex3f(-1.0f, 1.0f, 1.0f);

                    glVertex3f(-1.0f, -1.0f, -1.0f);
                    glVertex3f(1.0f, -1.0f, -1.0f);
                    glVertex3f(1.0f, -1.0f, 1.0f);
                    glVertex3f(-1.0f, -1.0f, 1.0f);
                    glEnd();

                    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                    glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y + camera.getViewportDimensions().y / 2,
                        camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);

                    planes[0] = camera.getFarVal() / (camera.getFarVal() - camera.getNearVal());
                    planes[1] = (camera.getFarVal() * camera.getNearVal()) / (camera.getFarVal() - camera.getNearVal()); //normally this is negated in a left handed coordinate system

                    glUniform2fv(getProgramUniformLocation(prog, "planes"), 1, planes);
                }

                glm::mat4 modelView = glm::scale(camera.getModelView() * node->getTransform(), volumeScale);
                
                glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 
                    1, false, glm::value_ptr(glm::scale(camera.getModelViewProjection() * node->getTransform(), volumeScale)));
                
                glUniformMatrix4fv(getProgramUniformLocation(prog, "modelView"), 
                    1, false, glm::value_ptr(modelView));
                
                switch(lightType) {
                case illGraphics::LightBase::Type::POINT:
                case illGraphics::LightBase::Type::SPOT:
                    glUniform3fv(getProgramUniformLocation(prog, "lightPosition"), 
                        1, glm::value_ptr(getTransformPosition(camera.getModelView() * node->getTransform())));

                    /*
                    if light volume intersects far plane, back face culling, otherwise front face culling
                    assuming the light center and attenuation end as sphere radius will suffice as a good test, 
                    unless your light is HUGE or the draw distance is TINY
                    in that case, you're doing it wrong!!!!
                    */
                    if(camera.getViewFrustum().m_far.distance(getTransformPosition(node->getTransform())) 
                            < static_cast<illGraphics::PointLight*>(light)->m_attenuationEnd) {
                        glCullFace(GL_BACK);
                    }
                    else {
                        glCullFace(GL_FRONT);
                    }

                    break;

                case illGraphics::LightBase::Type::DIRECTIONAL:
                    //TODO:
                    break;
                }

                //TODO: clearly I don't plan on using a box drawn in immediate mode for long, I will draw the light volume shapes soon with PROPER VBOs
                
                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                glBegin(GL_QUADS);
                glVertex3f(1.0f, -1.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, 1.0f);
                glVertex3f(-1.0f, -1.0f, 1.0f);

                glVertex3f(-1.0f, -1.0f, -1.0f);
                glVertex3f(-1.0f, 1.0f, -1.0f);
                glVertex3f(1.0f, 1.0f, -1.0f);
                glVertex3f(1.0f, -1.0f, -1.0f);

                glVertex3f(-1.0f, -1.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, 1.0f);
                glVertex3f(-1.0f, 1.0f, -1.0f);
                glVertex3f(-1.0f, -1.0f, -1.0f);

                glVertex3f(1.0f, -1.0f, -1.0f);
                glVertex3f(1.0f, 1.0f, -1.0f);
                glVertex3f(1.0f, 1.0f, 1.0f);
                glVertex3f(1.0f, -1.0f, 1.0f);

                glVertex3f(1.0f, 1.0f, 1.0f);
                glVertex3f(1.0f, 1.0f, -1.0f);
                glVertex3f(-1.0f, 1.0f, -1.0f);
                glVertex3f(-1.0f, 1.0f, 1.0f);

                glVertex3f(-1.0f, -1.0f, -1.0f);
                glVertex3f(1.0f, -1.0f, -1.0f);
                glVertex3f(1.0f, -1.0f, 1.0f);
                glVertex3f(-1.0f, -1.0f, 1.0f);
                glEnd();

                //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }
    }

    if(m_debugOcclusion) {
        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y, camera.getViewportDimensions().x, camera.getViewportDimensions().y);
    }
}

void DeferredShadingBackendGl3_3::renderFinish() {
    //Combine accumulation buffers
    //TODO: heh for now just do this with the fixed function pipeline... Later write a shader since fixed function pipeline is gone in GL 3.3
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0.0f, 1.0f,
        0.0f, 1.0f,
        -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glUseProgram(0);

    glActiveTexture(GL_TEXTURE1);
    glDisable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE2);
    glDisable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE3);
    glDisable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);

    glColor3f(1.0f, 1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_DIFFUSE_ACCUMULATION]);

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

    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_SPECULAR_ACCUMULATION]);

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

void renderDebugLightPos(const glm::vec3& position) {
    glPushMatrix();

    glTranslatef(position.x, position.y, position.z);

    glBegin(GL_LINES);
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);

    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, 0.0f, 0.0f);

    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);

    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -1.0f, 0.0f);

    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);

    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -1.0f);
    glEnd();

    glPopMatrix();
}

void renderDebugPointLight(const glm::vec3& position, const illGraphics::PointLight& light) {
    renderDebugLightPos(position);

    glPushMatrix();

    glTranslatef(position.x, position.y, position.z);
    glScalef(light.m_attenuationEnd, light.m_attenuationEnd, light.m_attenuationEnd);

    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.1f);

    glBegin(GL_LINE_LOOP);
    glVertex3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -1.0f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, -1.0f, 0.0f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -1.0f);
    glEnd();

    glPopMatrix();
}

void renderDebugSpotLight(const glm::vec3& position, const illGraphics::SpotLight& light) {
    renderDebugLightPos(position);
}

void DeferredShadingBackendGl3_3::renderDebugLights(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera) {
    //manually go in to 3D mode while using fixed function pipeline in older OpenGL
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMultMatrixf(glm::value_ptr(camera.getProjection()));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMultMatrixf(glm::value_ptr(camera.getModelView()));

    glUseProgram(0);

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE1);
    glDisable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE2);
    glDisable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE3);
    glDisable(GL_TEXTURE_2D);

    for(auto lightTypeIter = renderQueues.m_lights.begin(); lightTypeIter != renderQueues.m_lights.end(); lightTypeIter++) {
        illGraphics::LightBase::Type lightType = lightTypeIter->first;
        auto& lights = lightTypeIter->second;

        for(auto lightIter = lights.begin(); lightIter != lights.end(); lightIter++) {
            illGraphics::LightBase * light = lightIter->first;
            auto& lightNodes = lightIter->second;

            for(auto nodeIter = lightNodes.begin(); nodeIter != lightNodes.end(); nodeIter++) {
                const illRendererCommon::LightNode * node = *nodeIter;

                switch(lightType) {
                case illGraphics::LightBase::Type::POINT:
                    renderDebugPointLight(getTransformPosition(node->getTransform()), *static_cast<illGraphics::PointLight *>(light));
                    break;

                case illGraphics::LightBase::Type::SPOT:
                    renderDebugPointLight(getTransformPosition(node->getTransform()), *static_cast<illGraphics::SpotLight *>(light));
                    break;
                }
            }
        }
    }
}

void renderNodeBounds(const illRendererCommon::GraphicsNode * node) {
    glColor4f(1.0f, 1.0f, 1.0f, 0.1f);

    Box<> bounds = node->getWorldBoundingVolume();

    glBegin(GL_LINE_LOOP);
    glVertex3f(bounds.m_min.x, bounds.m_min.y, bounds.m_min.z);
    glVertex3f(bounds.m_max.x, bounds.m_min.y, bounds.m_min.z);
    glVertex3f(bounds.m_max.x, bounds.m_max.y, bounds.m_min.z);
    glVertex3f(bounds.m_min.x, bounds.m_max.y, bounds.m_min.z);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3f(bounds.m_min.x, bounds.m_min.y, bounds.m_max.z);
    glVertex3f(bounds.m_max.x, bounds.m_min.y, bounds.m_max.z);
    glVertex3f(bounds.m_max.x, bounds.m_max.y, bounds.m_max.z);
    glVertex3f(bounds.m_min.x, bounds.m_max.y, bounds.m_max.z);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(bounds.m_min.x, bounds.m_min.y, bounds.m_min.z);
    glVertex3f(bounds.m_min.x, bounds.m_min.y, bounds.m_max.z);

    glVertex3f(bounds.m_max.x, bounds.m_min.y, bounds.m_min.z);
    glVertex3f(bounds.m_max.x, bounds.m_min.y, bounds.m_max.z);

    glVertex3f(bounds.m_max.x, bounds.m_max.y, bounds.m_min.z);
    glVertex3f(bounds.m_max.x, bounds.m_max.y, bounds.m_max.z);

    glVertex3f(bounds.m_min.x, bounds.m_max.y, bounds.m_min.z);
    glVertex3f(bounds.m_min.x, bounds.m_max.y, bounds.m_max.z);
    glEnd();
}

void DeferredShadingBackendGl3_3::renderDebugBounds(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera) {
    //manually go in to 3D mode while using fixed function pipeline in older OpenGL
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMultMatrixf(glm::value_ptr(camera.getProjection()));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMultMatrixf(glm::value_ptr(camera.getModelView()));

    glUseProgram(0);

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE1);
    glDisable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE2);
    glDisable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE3);
    glDisable(GL_TEXTURE_2D);

    for(auto shaderIter = renderQueues.m_solidStaticMeshes.begin(); shaderIter != renderQueues.m_solidStaticMeshes.end(); shaderIter++) {
        auto& materials = shaderIter->second;
        
        for(auto materialIter = materials.begin(); materialIter != materials.end(); materialIter++) {            
            auto& meshes = materialIter->second;

            for(auto meshIter = meshes.begin(); meshIter != meshes.end(); meshIter++) {
                auto& meshNodes = meshIter->second;

                for(auto nodeIter = meshNodes.begin(); nodeIter !=  meshNodes.end(); nodeIter++) {
                    const illRendererCommon::RenderQueues::StaticMeshInfo& meshInfo = *nodeIter;

                    if(m_debugOcclusion) {
                        glMatrixMode(GL_PROJECTION);
                        glLoadIdentity();

                        glMultMatrixf(glm::value_ptr(m_occlusionCamera->getProjection()));

                        glMatrixMode(GL_MODELVIEW);
                        glLoadIdentity();

                        glMultMatrixf(glm::value_ptr(m_occlusionCamera->getModelView()));

                        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y,
                            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
                        
                        renderNodeBounds(nodeIter->m_node);
                        
                        glMatrixMode(GL_PROJECTION);
                        glLoadIdentity();

                        glMultMatrixf(glm::value_ptr(camera.getProjection()));

                        glMatrixMode(GL_MODELVIEW);
                        glLoadIdentity();

                        glMultMatrixf(glm::value_ptr(camera.getModelView()));

                        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y + camera.getViewportDimensions().y / 2,
                            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
                    }

                    renderNodeBounds(nodeIter->m_node);
                }
            }
        }
    }

    for(auto lightTypeIter = renderQueues.m_lights.begin(); lightTypeIter != renderQueues.m_lights.end(); lightTypeIter++) {        
        auto& lights = lightTypeIter->second;

        for(auto lightIter = lights.begin(); lightIter != lights.end(); lightIter++) {            
            auto& lightNodes = lightIter->second;

            for(auto nodeIter = lightNodes.begin(); nodeIter != lightNodes.end(); nodeIter++) {
                const illRendererCommon::LightNode * node = *nodeIter;

                renderNodeBounds(node);
            }
        }
    }
}

void DeferredShadingBackendGl3_3::render(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera) {
    //disable depth mask
    glDepthMask(GL_FALSE);

    //enable color mask
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    //enable depth func equal
    glDepthFunc(GL_EQUAL);
    
    renderGbuffer(renderQueues, camera);

    if(m_debugMode == DebugMode::NONE || m_debugMode == DebugMode::DIFFUSE_ACCUMULATION || m_debugMode == DebugMode::SPECULAR_ACCUMULATION) {
        //set up the FBO for rendering to the diffuse buffer
        GLenum mrt[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTextures[REN_DIFFUSE_ACCUMULATION], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, 0, 0);
        glDrawBuffers(1, mrt);
        glClear(GL_COLOR_BUFFER_BIT);

        //disable depth test
        glDisable(GL_DEPTH_TEST);

        renderAmbientPass(renderQueues, camera);

        //set up the FBO for rendering to the specular buffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTextures[REN_SPECULAR_ACCUMULATION], 0);
        glClear(GL_COLOR_BUFFER_BIT);

        //enable additive blend
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        //enable depth test func less equal
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        renderEmissivePass(renderQueues, camera);

        //set up the FBO for rendering to both the specular and diffuse buffers
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTextures[REN_DIFFUSE_ACCUMULATION], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_renderTextures[REN_SPECULAR_ACCUMULATION], 0);
        glDrawBuffers(2, mrt);
         
        //TODO: for now disable depth test
        glDisable(GL_DEPTH_TEST);

        renderLights(renderQueues, camera);

        //TODO: forward rendering of blended stuff
        //TODO: post processing

        if(m_debugMode != DebugMode::DIFFUSE_ACCUMULATION && m_debugMode != DebugMode::SPECULAR_ACCUMULATION) {
            //disable face culling
            glDisable(GL_CULL_FACE);

            //disable depth test
            glDisable(GL_DEPTH_TEST);

            //draw to screen again
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            renderFinish();
        }
        else {
            switch(m_debugMode) {
            case DebugMode::DIFFUSE_ACCUMULATION:
                renderDebugTexture(m_renderTextures[REN_DIFFUSE_ACCUMULATION]);
                break;

            case DebugMode::SPECULAR_ACCUMULATION:
                renderDebugTexture(m_renderTextures[REN_SPECULAR_ACCUMULATION]);
                break;
            }
        }
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

    if(m_debugLights) {
        renderDebugLights(renderQueues, camera);
    }

    if(m_debugBounds) {
        renderDebugBounds(renderQueues, camera);
    }
}

}