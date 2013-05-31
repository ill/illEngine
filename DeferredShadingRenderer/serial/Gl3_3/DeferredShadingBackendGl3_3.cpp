#include <glm/gtc/type_ptr.hpp>

#include "DeferredShadingBackendGl3_3.h"
#include "glCommon/glLogging.h"
#include "glCommon/serial/glUtil.h"
#include "Graphics/serial/Camera/Camera.h"

#include "RendererCommon/serial/StaticMeshNode.h"
#include "RendererCommon/serial/LightNode.h"

#include "Util/Geometry/Iterators/ConvexMeshIterator.h"

void renderSceneDebug(const GridVolume3D<>& gridVolume);
void renderMeshEdgeListDebug(const MeshEdgeList<>& edgeList);

//#define VERIFY_RENDER_STATE

inline GLenum getPrimitiveType(MeshData<>::PrimitiveGroup::Type type) {
    switch(type) {
    case MeshData<>::PrimitiveGroup::Type::LINES:
        return GL_LINES;
    case MeshData<>::PrimitiveGroup::Type::LINE_LOOP:
        return GL_LINE_LOOP;
    case MeshData<>::PrimitiveGroup::Type::POINTS:
        return GL_POINTS;
    case MeshData<>::PrimitiveGroup::Type::TRIANGLES:
        return GL_TRIANGLES;
    case MeshData<>::PrimitiveGroup::Type::TRIANGLE_FAN:
        return GL_TRIANGLE_FAN;
    case MeshData<>::PrimitiveGroup::Type::TRIANGLE_STRIP:
        return GL_TRIANGLE_STRIP;
    default:
        LOG_FATAL_ERROR("Unknown Primitive Type %u", type);
        return 0;
    }
}

namespace illDeferredShadingRenderer {

void DeferredShadingBackendGl3_3::initialize(const glm::uvec2 screenResolution, illGraphics::ShaderProgramManager * shaderProgramManager) {
    uninitialize();

    glGenFramebuffers(1, &m_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);

    glActiveTexture(GL_TEXTURE0);

    glGenTextures(REN_LAST, m_renderTextures);
    
    glBindTexture(GL_TEXTURE_2D, m_renderTextures[REN_DEPTH]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, screenResolution.x, screenResolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);      //TODO: make sure GL_FLOAT is right for this
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    //glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);            //TODO: why do I have this?
    
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
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_renderTextures[REN_DEPTH], 0);
    
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
            fragShader->loadInternal(m_graphicsBackend, "shaders/deferredPhongLighting.frag", GL_FRAGMENT_SHADER, "#define POINT_LIGHT\n#define SPECULAR");

            shaders.push_back(RefCountPtr<illGraphics::Shader>(fragShader));
            m_deferredPointLightProgram.loadInternal(m_internalShaderProgramLoader, shaders);
        }

        //point light no specular
        {
            std::vector<RefCountPtr<illGraphics::Shader> > shaders;
            shaders.push_back(lightVertexShader);

            illGraphics::Shader * fragShader = new illGraphics::Shader();
            fragShader->loadInternal(m_graphicsBackend, "shaders/deferredPhongLighting.frag", GL_FRAGMENT_SHADER, "#define POINT_LIGHT");

            shaders.push_back(RefCountPtr<illGraphics::Shader>(fragShader));
            m_deferredPointLightNoSpecProgram.loadInternal(m_internalShaderProgramLoader, shaders);
        }

        //spot light
        {
            std::vector<RefCountPtr<illGraphics::Shader> > shaders;
            shaders.push_back(lightVertexShader);

            illGraphics::Shader * fragShader = new illGraphics::Shader();
            fragShader->loadInternal(m_graphicsBackend, "shaders/deferredPhongLighting.frag", GL_FRAGMENT_SHADER, "#define SPOT_LIGHT\n#define SPECULAR");

            shaders.push_back(RefCountPtr<illGraphics::Shader>(fragShader));
            m_deferredSpotLightProgram.loadInternal(m_internalShaderProgramLoader, shaders);
        }

        //spot light no specular 
        {
            std::vector<RefCountPtr<illGraphics::Shader> > shaders;
            shaders.push_back(lightVertexShader);

            illGraphics::Shader * fragShader = new illGraphics::Shader();
            fragShader->loadInternal(m_graphicsBackend, "shaders/deferredPhongLighting.frag", GL_FRAGMENT_SHADER, "#define SPOT_LIGHT");

            shaders.push_back(RefCountPtr<illGraphics::Shader>(fragShader));
            m_deferredSpotLightNoSpecProgram.loadInternal(m_internalShaderProgramLoader, shaders);
        }

        //point volume light
        {
            std::vector<RefCountPtr<illGraphics::Shader> > shaders;
            shaders.push_back(lightVertexShader);

            illGraphics::Shader * fragShader = new illGraphics::Shader();
            fragShader->loadInternal(m_graphicsBackend, "shaders/deferredPhongLighting.frag", GL_FRAGMENT_SHADER, "#define POINT_LIGHT\n#define VOLUME_LIGHT\n#define SPECULAR");

            shaders.push_back(RefCountPtr<illGraphics::Shader>(fragShader));
            m_deferredDirectionVolumeLightProgram.loadInternal(m_internalShaderProgramLoader, shaders);
        }

        //point volume light no specular
        {
            std::vector<RefCountPtr<illGraphics::Shader> > shaders;
            shaders.push_back(lightVertexShader);

            illGraphics::Shader * fragShader = new illGraphics::Shader();
            fragShader->loadInternal(m_graphicsBackend, "shaders/deferredPhongLighting.frag", GL_FRAGMENT_SHADER, "#define POINT_LIGHT\n#define VOLUME_LIGHT");

            shaders.push_back(RefCountPtr<illGraphics::Shader>(fragShader));
            m_deferredDirectionVolumeLightNoSpecProgram.loadInternal(m_internalShaderProgramLoader, shaders);
        }

        //direction volume light
        {
            std::vector<RefCountPtr<illGraphics::Shader> > shaders;
            shaders.push_back(lightVertexShader);

            illGraphics::Shader * fragShader = new illGraphics::Shader();
            fragShader->loadInternal(m_graphicsBackend, "shaders/deferredPhongLighting.frag", GL_FRAGMENT_SHADER, "#define DIRECTIONAL_LIGHT\n#define VOLUME_LIGHT\n#define SPECULAR");

            shaders.push_back(RefCountPtr<illGraphics::Shader>(fragShader));
            m_deferredDirectionVolumeLightProgram.loadInternal(m_internalShaderProgramLoader, shaders);
        }

        //direction volume light no specular
        {
            std::vector<RefCountPtr<illGraphics::Shader> > shaders;
            shaders.push_back(lightVertexShader);

            illGraphics::Shader * fragShader = new illGraphics::Shader();
            fragShader->loadInternal(m_graphicsBackend, "shaders/deferredPhongLighting.frag", GL_FRAGMENT_SHADER, "#define DIRECTIONAL_LIGHT\n#define VOLUME_LIGHT");

            shaders.push_back(RefCountPtr<illGraphics::Shader>(fragShader));
            m_deferredDirectionVolumeLightNoSpecProgram.loadInternal(m_internalShaderProgramLoader, shaders);
        }
    }
    
    m_volumeRenderProgram = shaderProgramManager->getResource(illGraphics::ShaderProgram::SHPRG_POSITIONS | illGraphics::ShaderProgram::SHPRG_FORWARD);

    //load the light volumes
    m_box.setFrontentDataInternal(new MeshData<>(Box<>(glm::vec3(-0.5f), glm::vec3(0.5f)), MF_POSITION)); 
    m_box.frontendBackendTransferInternal(m_graphicsBackend);

    //setup the query cache
    glGenQueries(100000, m_queryCacheTest);

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

void DeferredShadingBackendGl3_3::setupGbuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTextures[REN_NORMAL], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_renderTextures[REN_DIFFUSE], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_renderTextures[REN_SPECULAR], 0);

    GLenum mrt[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, mrt);
}

void DeferredShadingBackendGl3_3::setupFrame() {
    //clear the render target datas
    setupGbuffer();
    
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0xff);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_currentQuery = 0;
}

void DeferredShadingBackendGl3_3::setupViewport(const illGraphics::Camera& camera) {
    glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y, camera.getViewportDimensions().x, camera.getViewportDimensions().y);
    
    //prepare for depth passes

    //disable blend
    glDisable(GL_BLEND);

    //disable color mask
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    //enable depth func less
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //set up backface culling
    glCullFace(GL_BACK);

    setupGbuffer();
}

void DeferredShadingBackendGl3_3::retreiveCellQueries(std::unordered_map<size_t, Array<uint64_t>>& lastViewedFrames, uint64_t lastFrameCounter, 
        uint64_t successDuration, uint64_t failureDuration, uint64_t randomAddMax) {    
    for(auto iter = m_cellQueries.begin(); iter != m_cellQueries.end(); iter++) {
        CellQuery& cellQuery = *iter;

        if(m_performCull) {
            GLint result;
            glGetQueryObjectiv(cellQuery.m_query, GL_QUERY_RESULT, &result);
            
            lastViewedFrames.at(cellQuery.m_viewport)[cellQuery.m_cellArrayIndex] = 
                codeFrame(lastFrameCounter + (result != 0 ? successDuration : failureDuration) + (std::rand() % (randomAddMax + 1))) 
                | encodeVisible(result != 0);
        }

        //TODO: figure out if deleting queries as soon as I used them is bad for performance, maintaining my own pool might be kindof useless though since GL does it too
        //I figured it out, it's very bad for performance
        //glDeleteQueries(1, &cellQuery.m_query);
    }

    m_cellQueries.clear();
}

void DeferredShadingBackendGl3_3::retreiveNodeQueries(uint64_t lastFrameCounter) {
    for(auto iter = m_nodeQueries.begin(); iter != m_nodeQueries.end(); iter++) {
        NodeQuery& nodeQuery = *iter;

        if(m_performCull) {
            GLint result;
            glGetQueryObjectiv(nodeQuery.m_query, GL_QUERY_RESULT, &result);

            if(!result) {
                nodeQuery.m_node->setLastNonvisibleFrame(nodeQuery.m_viewport, lastFrameCounter);
            }
        }

        //TODO: figure out if deleting queries as soon as I used them is bad for performance, maintaining my own pool might be kindof useless though since GL does it too
        glDeleteQueries(1, &nodeQuery.m_query);
    }

    m_nodeQueries.clear();
}

void DeferredShadingBackendGl3_3::setupQuery() {
    //just disable face culling for this super simple box being drawn
    glDisable(GL_CULL_FACE);

    //disable depth write
    //glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);    //debug draw some red to the normals buffer
    glDepthMask(GL_FALSE);

    GLuint prog = getProgram(*m_volumeRenderProgram.get());

    glUseProgram(prog);

    //bind VBO
    {
        GLuint buffer = *((GLuint *) m_box.getMeshBackendData() + 0);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
    }

    //bind IBO
    {
        GLuint buffer = *((GLuint *) m_box.getMeshBackendData() + 1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    }

    //setup positions
    GLint posAttrib = getProgramAttribLocation(prog, "positionIn");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, (GLsizei) m_box.getMeshFrontentData()->getVertexSize(), (char *)NULL + m_box.getMeshFrontentData()->getPositionOffset());
}

void DeferredShadingBackendGl3_3::endQuery() {
    glDisableVertexAttribArray(getProgramAttribLocation(getProgram(*m_volumeRenderProgram.get()), "positionIn"));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void renderQueryBox(const illGraphics::Camera& camera, const illGraphics::Mesh& boxMesh, GLuint program, GLuint query, const glm::vec3& boxCenter, const glm::vec3& boxSize) {
    glm::mat4 boxTransform = glm::scale(glm::translate(boxCenter), boxSize);

    glUniformMatrix4fv(getProgramUniformLocation(program, "modelViewProjection"), 1, false, glm::value_ptr(camera.getModelViewProjection() * boxTransform));

    glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, query);

#ifdef VERIFY_RENDER_STATE
    /**
    Gbuffer FBO bound
	MRT for normal, diffuse, specular is set
					
	viewport dimensions set
					
	blend disabled
					
	depth test enabled
	depth func less
					
	stencil test disabled
					
	cullface off
					
	depthmask false
	colormask false
					
	shader prog: volume rendering
					
	querybox mesh VBO bound
	querybox mesh IBO bound			
	shader prog position attribute array enabled
	mesh position attrib array data set
    */
    {
        GLboolean val;
        glGetBooleanv(GL_BLEND, &val);
        assert(val == GL_FALSE);
    }

    {
        GLboolean val;
        glGetBooleanv(GL_DEPTH_TEST, &val);
        assert(val == GL_TRUE);
    }

    {
        GLint val;
        glGetIntegerv(GL_DEPTH_FUNC, &val);
        assert(val == GL_LESS);
    }

    {
        GLboolean val;
        glGetBooleanv(GL_STENCIL_TEST, &val);
        assert(val == GL_FALSE);
    }

    {
        GLboolean val;
        glGetBooleanv(GL_CULL_FACE, &val);
        assert(val == GL_FALSE);
    }

    {
        GLboolean val;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &val);
        assert(val == GL_FALSE);
    }

    {
        GLboolean val[4];
        glGetBooleanv(GL_COLOR_WRITEMASK, val);
        assert(val[0] == GL_FALSE && val[1] == GL_FALSE && val[2] == GL_FALSE && val[3] == GL_FALSE);
    }
#endif

    glDrawRangeElements(GL_TRIANGLES, 0, boxMesh.getMeshFrontentData()->getNumInd(), boxMesh.getMeshFrontentData()->getNumInd(), GL_UNSIGNED_SHORT, (char *)NULL);
    glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
}

void * DeferredShadingBackendGl3_3::occlusionQueryCell(const illGraphics::Camera& camera, const glm::vec3& cellCenter, const glm::vec3& cellSize,
        unsigned int cellArrayIndex, size_t viewport) {
    //generate the occlusion query for the cell
    if(!m_performCull) {
        return NULL;
    }

    m_cellQueries.emplace_back();
    
    //glGenQueries(1, &m_cellQueries.back().m_query);
    m_cellQueries.back().m_query = m_queryCacheTest[m_currentQuery++];
    m_cellQueries.back().m_cellArrayIndex = cellArrayIndex;
    m_cellQueries.back().m_viewport = viewport;

    if(m_debugOcclusion) {
        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y + camera.getViewportDimensions().y / 2,
            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
    }
    
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);    //debug draw some purple to the normals buffer

    renderQueryBox(camera, m_box, getProgram(*m_volumeRenderProgram.get()), m_cellQueries.back().m_query, cellCenter, cellSize);
    
    //returns a pointer to the query name in OpenGL, depthPass will just typecast this this
    return &m_cellQueries.back().m_query;
}

void * DeferredShadingBackendGl3_3::occlusionQueryNode(const illGraphics::Camera& camera, illRendererCommon::GraphicsNode * node, size_t viewport) {
    //generate the occlusion query for the cell
    if(!m_performCull) {
        return NULL;
    }

    m_nodeQueries.emplace_back();
    
    glGenQueries(1, &m_nodeQueries.back().m_query);
    m_nodeQueries.back().m_node = node;
    m_nodeQueries.back().m_viewport = viewport;

    if(m_debugOcclusion) {
        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y + camera.getViewportDimensions().y / 2,
            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
    }

    Box<> nodeBox = node->getWorldBoundingVolume();

    glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_TRUE);    //debug draw some blue to the normals buffer

    renderQueryBox(camera, m_box, getProgram(*m_volumeRenderProgram.get()), m_nodeQueries.back().m_query, nodeBox.getCenter(), nodeBox.getDimensions());
    
    //returns a pointer to the query name in OpenGL, depthPass will just typecast this this
    return &m_cellQueries.back().m_query;
}

void DeferredShadingBackendGl3_3::depthPass(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera, void * cellOcclusionQuery, size_t viewport) {
    //enable depth write
    glDepthMask(GL_TRUE);
    
    //enable backface culling
    glEnable(GL_CULL_FACE);

    if(cellOcclusionQuery && m_performCull) {
        glBeginConditionalRender(*(GLuint *) cellOcclusionQuery, GL_ANY_SAMPLES_PASSED/*_CONSERVATIVE*/);//TODO: Does conservative not work here?
    }

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
                    auto& node = *nodeIter;
                    
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
                    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, (GLsizei) mesh->getMeshFrontentData()->getVertexSize(), (char *)NULL + mesh->getMeshFrontentData()->getPositionOffset());                

                    //TODO: skinning

                    //DRAW!!!!
                    if(m_debugOcclusion) {
                        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y,
                            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);

                        glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 1, false, glm::value_ptr(m_occlusionCamera->getModelViewProjection() * node.m_node->getTransform()));

                        glDrawRangeElements(GL_TRIANGLES, 0, mesh->getMeshFrontentData()->getNumInd(), mesh->getMeshFrontentData()->getNumInd(), GL_UNSIGNED_SHORT, (char *)NULL);

                        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y + camera.getViewportDimensions().y / 2,
                            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
                    }

                    glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 1, false, glm::value_ptr(camera.getModelViewProjection() * node.m_node->getTransform()));

                    if(node.m_node->getOcclusionCull()) {
                        m_nodeQueries.emplace_back();
    
                        glGenQueries(1, &m_nodeQueries.back().m_query);
                        m_nodeQueries.back().m_node = node.m_node;
                        m_nodeQueries.back().m_viewport = viewport;

                        glBeginQuery(/*GL_SAMPLES_PASSED*/GL_ANY_SAMPLES_PASSED, m_nodeQueries.back().m_query);
                    }

#ifdef VERIFY_RENDER_STATE
                    /**
                    Gbuffer FBO bound
					MRT for normal, diffuse, specular is set
					
					viewport dimensions set
					
					blend disabled
					
					depth test enabled
					depth func less
					
					stencil test disabled
					
					cullface back
					cullface on
					
					depthmask true
					colormask false
					
					shader prog: depth pass for mesh
										
					mesh VBO bound
					mesh IBO bound
					shader prog position attribute array enabled
					mesh position attrib array data set
					
					shader prog modelViewProjection set to transform
                    */
                    {
                        GLboolean val;
                        glGetBooleanv(GL_BLEND, &val);
                        assert(val == GL_FALSE);
                    }

                    {
                        GLboolean val;
                        glGetBooleanv(GL_DEPTH_TEST, &val);
                        assert(val == GL_TRUE);
                    }

                    {
                        GLint val;
                        glGetIntegerv(GL_DEPTH_FUNC, &val);
                        assert(val == GL_LESS);
                    }

                    {
                        GLboolean val;
                        glGetBooleanv(GL_STENCIL_TEST, &val);
                        assert(val == GL_FALSE);
                    }

                    {
                        GLboolean val;
                        glGetBooleanv(GL_CULL_FACE, &val);
                        assert(val == GL_TRUE);
                    }

                    {
                        GLboolean val;
                        glGetBooleanv(GL_DEPTH_WRITEMASK, &val);
                        assert(val == GL_TRUE);
                    }

                    {
                        GLboolean val[4];
                        glGetBooleanv(GL_COLOR_WRITEMASK, val);
                        assert(val[0] == GL_FALSE && val[1] == GL_FALSE && val[2] == GL_FALSE && val[3] == GL_FALSE);
                    }
#endif

                    {
                        auto& primitiveGroup = mesh->getMeshFrontentData()->getPrimitiveGroup(node.m_primitiveGroup);

                        GLuint startInd = primitiveGroup.m_beginIndex;
                        GLuint numInd = primitiveGroup.m_numIndices;
                        GLuint endInd = startInd + numInd;
                        
                        glDrawRangeElements(getPrimitiveType(primitiveGroup.m_type), startInd, endInd, numInd, GL_UNSIGNED_SHORT, (char *)NULL + startInd * sizeof(uint16_t));
                    }
                                        
                    if(node.m_node->getOcclusionCull()) {                        
                        glEndQuery(/*GL_SAMPLES_PASSED*/GL_ANY_SAMPLES_PASSED);
                    }
                }
            }

            //TODO: disable skinning attrib
        }

        glDisableVertexAttribArray(posAttrib);
    }

    if(cellOcclusionQuery && m_performCull) {
        glEndConditionalRender();
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
                    const auto& meshInfo = *nodeIter;
                    
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
                    if(texCoordAttrib >= 0) {
                        glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 
                            (GLsizei) mesh->getMeshFrontentData()->getVertexSize(), (char *)NULL + mesh->getMeshFrontentData()->getTexCoordOffset());
                    }

                    //setup normals
                    glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 
                        (GLsizei) mesh->getMeshFrontentData()->getVertexSize(), (char *)NULL + mesh->getMeshFrontentData()->getNormalOffset());
                    
                    //setup tangents
                    if(tangentsAttrib >= 0) {
                        glVertexAttribPointer(tangentsAttrib, 3, GL_FLOAT, GL_FALSE, 
                            (GLsizei) mesh->getMeshFrontentData()->getVertexSize(), (char *)NULL + mesh->getMeshFrontentData()->getTangentOffset());

                        //setup bitangents
                        glVertexAttribPointer(bitangentsAttrib, 3, GL_FLOAT, GL_FALSE, 
                            (GLsizei) mesh->getMeshFrontentData()->getVertexSize(), (char *)NULL + mesh->getMeshFrontentData()->getBitangentOffset());
                    }

                    //TODO: skinning

                    //DRAW!!!!
                    if(m_debugOcclusion) {
                        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y,
                            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);

                        glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 
                            1, false, glm::value_ptr(m_occlusionCamera->getModelViewProjection() * meshInfo.m_meshInfo.m_node->getTransform()));

                        glUniformMatrix3fv(getProgramUniformLocation(prog, "normalMat"), 
                            1, false, glm::value_ptr(glm::mat3(m_occlusionCamera->getModelView() * meshInfo.m_meshInfo.m_node->getTransform())));

                        glDrawRangeElements(GL_TRIANGLES, 0, 
                            mesh->getMeshFrontentData()->getNumInd(), mesh->getMeshFrontentData()->getNumInd(), GL_UNSIGNED_SHORT, (char *)NULL);
                        
                        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y + camera.getViewportDimensions().y / 2,
                            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
                    }

                    glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 
                        1, false, glm::value_ptr(camera.getModelViewProjection() * meshInfo.m_meshInfo.m_node->getTransform()));

                    glUniformMatrix3fv(getProgramUniformLocation(prog, "normalMat"), 
                        1, false, glm::value_ptr(glm::mat3(camera.getModelView() * meshInfo.m_meshInfo.m_node->getTransform())));

#ifdef VERIFY_RENDER_STATE
                    /**
                    Gbuffer FBO bound
					MRT for normal, diffuse, specular is set
					
					viewport dimensions set
					
					blend disabled
					
					depth test enabled
					depth func equal
					
					stencil test disabled
					
					cullface back
					cullface on
					
					depthmask false
					colormask true
					
					shader prog: mesh rendering
										
					mesh VBO bound
					mesh IBO bound
					
					position attrib array
					normal attrib array
					tex coord array
					tangent array
					bitangent array
                    */
                    {
                        GLboolean val;
                        glGetBooleanv(GL_BLEND, &val);
                        assert(val == GL_FALSE);
                    }

                    {
                        GLboolean val;
                        glGetBooleanv(GL_DEPTH_TEST, &val);
                        assert(val == GL_TRUE);
                    }

                    {
                        GLint val;
                        glGetIntegerv(GL_DEPTH_FUNC, &val);
                        assert(val == GL_EQUAL);
                    }

                    {
                        GLboolean val;
                        glGetBooleanv(GL_STENCIL_TEST, &val);
                        assert(val == GL_FALSE);
                    }

                    {
                        GLboolean val;
                        glGetBooleanv(GL_CULL_FACE, &val);
                        assert(val == GL_TRUE);
                    }

                    {
                        GLboolean val;
                        glGetBooleanv(GL_DEPTH_WRITEMASK, &val);
                        assert(val == GL_FALSE);
                    }

                    {
                        GLboolean val[4];
                        glGetBooleanv(GL_COLOR_WRITEMASK, val);
                        assert(val[0] == GL_TRUE && val[1] == GL_TRUE && val[2] == GL_TRUE && val[3] == GL_TRUE);
                    }
#endif
                    {
                        auto& primitiveGroup = mesh->getMeshFrontentData()->getPrimitiveGroup(meshInfo.m_meshInfo.m_primitiveGroup);

                        GLuint startInd = primitiveGroup.m_beginIndex;
                        GLuint numInd = primitiveGroup.m_numIndices;
                        GLuint endInd = startInd + numInd;

                        //LOG_DEBUG("Primitive Group %u", meshInfo.m_meshInfo.m_primitiveGroup);

                        glDrawRangeElements(getPrimitiveType(primitiveGroup.m_type), startInd, endInd, numInd, GL_UNSIGNED_SHORT, (char *)NULL + startInd * sizeof(uint16_t));
                    }
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

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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

void DeferredShadingBackendGl3_3::renderLights(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera, size_t viewport) {
    if(m_debugOcclusion) {
        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y + camera.getViewportDimensions().y / 2,
            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
    }
    
    if(m_stencilLightingPass) {    
        glEnable(GL_STENCIL_TEST);
    }
    
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

        bool hasSpecular = true;

        switch(lightType) {
        case illGraphics::LightBase::Type::POINT:
            prog = getProgram(m_deferredPointLightProgram);
            lightVolume = &m_pointLightVolume;
            break;

        case illGraphics::LightBase::Type::POINT_NOSPECULAR:
            prog = getProgram(m_deferredPointLightNoSpecProgram);
            lightVolume = &m_pointLightVolume;
            hasSpecular = false;
            break;

        case illGraphics::LightBase::Type::SPOT:
            prog = getProgram(m_deferredSpotLightProgram);
            lightVolume = &m_spotLightVolume;
            break;

        case illGraphics::LightBase::Type::SPOT_NOSPECULAR:
            prog = getProgram(m_deferredSpotLightNoSpecProgram);
            lightVolume = &m_spotLightVolume;
            hasSpecular = false;
            break;

        case illGraphics::LightBase::Type::POINT_VOLUME:
            prog = getProgram(m_deferredPointVolumeLightProgram);
            lightVolume = &m_quad;
            break;

        case illGraphics::LightBase::Type::POINT_VOLUME_NOSPECULAR:
            prog = getProgram(m_deferredPointVolumeLightNoSpecProgram);
            lightVolume = &m_quad;
            hasSpecular = false;
            break;

        case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME:
            prog = getProgram(m_deferredDirectionVolumeLightProgram);
            lightVolume = &m_quad;
            break;

        case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME_NOSPECULAR:
            prog = getProgram(m_deferredDirectionVolumeLightNoSpecProgram);
            lightVolume = &m_quad;
            hasSpecular = false;
            break;
        }

        glUseProgram(prog);

        glUniform2fv(getProgramUniformLocation(prog, "planes"), 1, planes);
        
        glUniform1i(getProgramUniformLocation(prog, "depthBuffer"), 0);
        glUniform1i(getProgramUniformLocation(prog, "normalBuffer"), 1);
        glUniform1i(getProgramUniformLocation(prog, "diffuseBuffer"), 2);

        if(hasSpecular) {
            glUniform1i(getProgramUniformLocation(prog, "specularBuffer"), 3);
        }

        GLint posAttrib = getProgramAttribLocation(prog, "positionIn");
        glEnableVertexAttribArray(posAttrib);

        for(auto lightIter = lights.begin(); lightIter != lights.end(); lightIter++) {
            illGraphics::LightBase * light = lightIter->first;
            auto& lightNodes = lightIter->second;

            glUniform1f(getProgramUniformLocation(prog, "intensity"), light->m_intensity);
            glUniform3fv(getProgramUniformLocation(prog, "lightColor"), 1, glm::value_ptr(light->m_color));

            glm::vec3 volumeScale;

            switch(lightType) {
            case illGraphics::LightBase::Type::POINT:
            case illGraphics::LightBase::Type::POINT_NOSPECULAR:
                glUniform1f(getProgramUniformLocation(prog, "attenuationStart"), 
                    static_cast<illGraphics::PointLight*>(light)->m_attenuationStart);

                glUniform1f(getProgramUniformLocation(prog, "attenuationEnd"), 
                    static_cast<illGraphics::PointLight*>(light)->m_attenuationEnd);

                volumeScale = glm::vec3(static_cast<illGraphics::PointLight*>(light)->m_attenuationEnd) * 2.0f;

                //bind VBO
                {
                    GLuint buffer = *((GLuint *) m_box.getMeshBackendData() + 0);
                    glBindBuffer(GL_ARRAY_BUFFER, buffer);
                }

                //bind IBO
                {
                    GLuint buffer = *((GLuint *) m_box.getMeshBackendData() + 1);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
                }

                glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, (GLsizei) m_box.getMeshFrontentData()->getVertexSize(), (char *)NULL + m_box.getMeshFrontentData()->getPositionOffset());

                break;

            case illGraphics::LightBase::Type::SPOT:
            case illGraphics::LightBase::Type::SPOT_NOSPECULAR:
                glUniform1f(getProgramUniformLocation(prog, "attenuationStart"), 
                    static_cast<illGraphics::SpotLight*>(light)->m_attenuationStart);

                glUniform1f(getProgramUniformLocation(prog, "attenuationEnd"), 
                    static_cast<illGraphics::SpotLight*>(light)->m_attenuationEnd);

                glUniform1f(getProgramUniformLocation(prog, "coneStart"), 
                    static_cast<illGraphics::SpotLight*>(light)->m_coneStart);

                glUniform1f(getProgramUniformLocation(prog, "coneEnd"), 
                    static_cast<illGraphics::SpotLight*>(light)->m_coneEnd);

                volumeScale = glm::vec3(static_cast<illGraphics::SpotLight*>(light)->m_attenuationEnd) * 2.0f;

                //TODO: use a cone shape
                //bind VBO
                {
                    GLuint buffer = *((GLuint *) m_box.getMeshBackendData() + 0);
                    glBindBuffer(GL_ARRAY_BUFFER, buffer);
                }

                //bind IBO
                {
                    GLuint buffer = *((GLuint *) m_box.getMeshBackendData() + 1);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
                }

                glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, (GLsizei) m_box.getMeshFrontentData()->getVertexSize(), (char *)NULL + m_box.getMeshFrontentData()->getPositionOffset());

                break;

            case illGraphics::LightBase::Type::POINT_VOLUME:
            case illGraphics::LightBase::Type::POINT_VOLUME_NOSPECULAR:
            case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME:
            case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME_NOSPECULAR:

                if(lightType == illGraphics::LightBase::Type::POINT_VOLUME || lightType == illGraphics::LightBase::Type::POINT_VOLUME_NOSPECULAR) {
                    glUniform3fv(getProgramUniformLocation(prog, "lightPosition"), 
                        1, glm::value_ptr(glm::mat3(camera.getModelView()) * static_cast<illGraphics::VolumeLight*>(light)->m_vector));
                }
                else {
                    glUniform3fv(getProgramUniformLocation(prog, "lightDirection"), 1,
                        glm::value_ptr(glm::mat3(camera.getModelView()) * static_cast<illGraphics::VolumeLight*>(light)->m_vector));
                }                
                                
                //bind VBO
                {
                    GLuint buffer = *((GLuint *) m_box.getMeshBackendData() + 0);
                    glBindBuffer(GL_ARRAY_BUFFER, buffer);
                }

                //bind IBO
                {
                    GLuint buffer = *((GLuint *) m_box.getMeshBackendData() + 1);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
                }

                glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, (GLsizei) m_box.getMeshFrontentData()->getVertexSize(), (char *)NULL + m_box.getMeshFrontentData()->getPositionOffset());

                //transform the planes into eye space
                {
                    glm::vec4 eyePlanes[illGraphics::MAX_LIGHT_VOLUME_PLANES];

                    for(size_t plane = 0; plane < illGraphics::MAX_LIGHT_VOLUME_PLANES; plane++) {
                        Plane<> eyePlane = static_cast<illGraphics::VolumeLight*>(light)->m_planes[plane].transform(/*glm::transpose(glm::inverse(*/camera.getModelView()/*))*/);
                        eyePlanes[plane] = glm::vec4(eyePlane.m_normal, eyePlane.m_distance);
                    }

                    glUniform4fv(getProgramUniformLocation(prog, "attenuationPlanes"), 12, glm::value_ptr(eyePlanes[0]));
                }

                glUniform1fv(getProgramUniformLocation(prog, "attenuationStarts"), 12, static_cast<illGraphics::VolumeLight*>(light)->m_planeFalloff);
                
                break;
            }

            for(auto nodeIter = lightNodes.begin(); nodeIter != lightNodes.end(); nodeIter++) {
                auto node = *nodeIter;
                
                if(lightType == illGraphics::LightBase::Type::SPOT || lightType == illGraphics::LightBase::Type::SPOT_NOSPECULAR) {
                    glUniform3fv(getProgramUniformLocation(prog, "lightDirection"), 1,
                        glm::value_ptr(glm::mat3(camera.getModelView() * node->getTransform()) * glm::vec3(0.0f, 0.0f, -1.0f)));
                }

                if(m_debugOcclusion) {
                    planes[0] = m_occlusionCamera->getFarVal() / (m_occlusionCamera->getFarVal() - m_occlusionCamera->getNearVal());
                    planes[1] = (m_occlusionCamera->getFarVal() * m_occlusionCamera->getNearVal()) / (m_occlusionCamera->getFarVal() - m_occlusionCamera->getNearVal()); //normally this is negated in a left handed coordinate system

                    glUniform2fv(getProgramUniformLocation(prog, "planes"), 1, planes);

                    glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y,
                        camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);

                    glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 
                        1, false, glm::value_ptr(glm::scale(m_occlusionCamera->getModelViewProjection() * node->getTransform(), volumeScale)));
                
                    glUniformMatrix4fv(getProgramUniformLocation(prog, "modelView"), 
                        1, false, glm::value_ptr(glm::scale(m_occlusionCamera->getModelView() * node->getTransform(), volumeScale)));
                
                    switch(lightType) {
                    case illGraphics::LightBase::Type::POINT:
                    case illGraphics::LightBase::Type::POINT_NOSPECULAR:
                    case illGraphics::LightBase::Type::SPOT:
                    case illGraphics::LightBase::Type::SPOT_NOSPECULAR:
                        glUniform3fv(getProgramUniformLocation(prog, "lightPosition"), 
                            1, glm::value_ptr(getTransformPosition(m_occlusionCamera->getModelView() * node->getTransform())));
                        
                        glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 
                            1, false, glm::value_ptr(glm::scale(m_occlusionCamera->getModelViewProjection() * node->getTransform(), volumeScale)));
                
                        glUniformMatrix4fv(getProgramUniformLocation(prog, "modelView"), 
                            1, false, glm::value_ptr(glm::scale(m_occlusionCamera->getModelView() * node->getTransform(), volumeScale)));

                        break;

                    case illGraphics::LightBase::Type::POINT_VOLUME:
                    case illGraphics::LightBase::Type::POINT_VOLUME_NOSPECULAR:
                    case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME:
                    case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME_NOSPECULAR:

                        {
                            glm::mat4 centerTransform = glm::translate(node->getWorldBoundingVolume().getCenter());

                            glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 
                                1, false, glm::value_ptr(glm::scale(m_occlusionCamera->getModelViewProjection() * centerTransform, node->getWorldBoundingVolume().getDimensions())));
                
                            glUniformMatrix4fv(getProgramUniformLocation(prog, "modelView"), 
                                1, false, glm::value_ptr(glm::scale(m_occlusionCamera->getModelView() * centerTransform, node->getWorldBoundingVolume().getDimensions())));

                            break;
                        }

                        break;
                    }

                    if(lightType == illGraphics::LightBase::Type::POINT_VOLUME
                        || lightType == illGraphics::LightBase::Type::POINT_VOLUME_NOSPECULAR
                        || lightType == illGraphics::LightBase::Type::DIRECTIONAL_VOLUME
                        || lightType == illGraphics::LightBase::Type::DIRECTIONAL_VOLUME_NOSPECULAR) {

                        if(lightType == illGraphics::LightBase::Type::POINT_VOLUME || lightType == illGraphics::LightBase::Type::POINT_VOLUME_NOSPECULAR) {
                            glUniform3fv(getProgramUniformLocation(prog, "lightPosition"), 
                                1, glm::value_ptr(glm::mat3(m_occlusionCamera->getModelView()) * static_cast<illGraphics::VolumeLight*>(light)->m_vector));
                        }
                        else {
                            glUniform3fv(getProgramUniformLocation(prog, "lightDirection"), 1,
                                glm::value_ptr(glm::mat3(m_occlusionCamera->getModelView()) * static_cast<illGraphics::VolumeLight*>(light)->m_vector));
                        }                
                                
                        //bind VBO
                        {
                            GLuint buffer = *((GLuint *) m_box.getMeshBackendData() + 0);
                            glBindBuffer(GL_ARRAY_BUFFER, buffer);
                        }

                        //bind IBO
                        {
                            GLuint buffer = *((GLuint *) m_box.getMeshBackendData() + 1);
                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
                        }

                        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, (GLsizei) m_box.getMeshFrontentData()->getVertexSize(), (char *)NULL + m_box.getMeshFrontentData()->getPositionOffset());

                        //transform the planes into eye space
                        {
                            glm::vec4 eyePlanes[illGraphics::MAX_LIGHT_VOLUME_PLANES];

                            for(size_t plane = 0; plane < illGraphics::MAX_LIGHT_VOLUME_PLANES; plane++) {
                                Plane<> eyePlane = static_cast<illGraphics::VolumeLight*>(light)->m_planes[plane].transform(/*glm::transpose(glm::inverse(*/m_occlusionCamera->getModelView()/*))*/);
                                eyePlanes[plane] = glm::vec4(eyePlane.m_normal, eyePlane.m_distance);
                            }

                            glUniform4fv(getProgramUniformLocation(prog, "attenuationPlanes"), 12, glm::value_ptr(eyePlanes[0]));
                        }

                        glUniform1fv(getProgramUniformLocation(prog, "attenuationStarts"), 12, static_cast<illGraphics::VolumeLight*>(light)->m_planeFalloff); 
                    }

                    //render stencil prepass

                    GLuint query = 0;

                    if(m_stencilLightingPass) {
                        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                                                
                        glEnable(GL_DEPTH_TEST);
                        glDisable(GL_CULL_FACE);
                        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    
                        glStencilFunc(GL_ALWAYS, 0, 0x00);

                        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
                        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

                        glUniform1i(getProgramUniformLocation(prog, "noLighting"), 1);
  
                        glDrawRangeElements(GL_TRIANGLES, 0, m_box.getMeshFrontentData()->getNumInd(), m_box.getMeshFrontentData()->getNumInd(), GL_UNSIGNED_SHORT, (char *)NULL);
                    }

                    /*
                    if light volume intersects far plane, back face culling, otherwise front face culling
                    assuming the light center and attenuation end as sphere radius will suffice as a good test, 
                    unless your light is HUGE or the draw distance is TINY
                    in that case, you're doing it wrong!!!!
                    */

                    switch(lightType) {
                    case illGraphics::LightBase::Type::POINT:
                    case illGraphics::LightBase::Type::POINT_NOSPECULAR:
                        if(m_occlusionCamera->getViewFrustum().m_far.distance(getTransformPosition(node->getTransform())) 
                                < static_cast<illGraphics::PointLight*>(light)->m_attenuationEnd) {
                            glCullFace(GL_BACK);
                        }
                        else {
                            glCullFace(GL_FRONT);
                        }
                        break;

                    case illGraphics::LightBase::Type::SPOT:
                    case illGraphics::LightBase::Type::SPOT_NOSPECULAR:                        
                        if(m_occlusionCamera->getViewFrustum().m_far.distance(getTransformPosition(node->getTransform())) 
                                < static_cast<illGraphics::SpotLight*>(light)->m_attenuationEnd) {
                            glCullFace(GL_BACK);
                        }
                        else {
                            glCullFace(GL_FRONT);
                        }
                        break;

                    case illGraphics::LightBase::Type::POINT_VOLUME:
                    case illGraphics::LightBase::Type::POINT_VOLUME_NOSPECULAR:
                    case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME:
                    case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME_NOSPECULAR:
                        if(m_occlusionCamera->getViewFrustum().m_far.distance(getTransformPosition(node->getTransform())) 
                                < static_cast<illGraphics::SpotLight*>(light)->m_attenuationEnd) {
                            glCullFace(GL_BACK);
                        }
                        else {
                            glCullFace(GL_FRONT);
                        }
                        break;
                    }
                    
                    //render light
                    glDisable(GL_DEPTH_TEST);
                    glEnable(GL_CULL_FACE);
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

                    if(m_stencilLightingPass) {
                        glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
                        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                    }

                    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                    glUniform1i(getProgramUniformLocation(prog, "noLighting"), 0);

                    glDrawRangeElements(GL_TRIANGLES, 0, m_box.getMeshFrontentData()->getNumInd(), m_box.getMeshFrontentData()->getNumInd(), GL_UNSIGNED_SHORT, (char *)NULL);
                    
                    glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y + camera.getViewportDimensions().y / 2,
                        camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);

                    planes[0] = camera.getFarVal() / (camera.getFarVal() - camera.getNearVal());
                    planes[1] = (camera.getFarVal() * camera.getNearVal()) / (camera.getFarVal() - camera.getNearVal()); //normally this is negated in a left handed coordinate system

                    glUniform2fv(getProgramUniformLocation(prog, "planes"), 1, planes);

                    if(lightType == illGraphics::LightBase::Type::POINT_VOLUME
                        || lightType == illGraphics::LightBase::Type::POINT_VOLUME_NOSPECULAR
                        || lightType == illGraphics::LightBase::Type::DIRECTIONAL_VOLUME
                        || lightType == illGraphics::LightBase::Type::DIRECTIONAL_VOLUME_NOSPECULAR) {

                        if(lightType == illGraphics::LightBase::Type::POINT_VOLUME || lightType == illGraphics::LightBase::Type::POINT_VOLUME_NOSPECULAR) {
                            glUniform3fv(getProgramUniformLocation(prog, "lightPosition"), 
                                1, glm::value_ptr(glm::mat3(camera.getModelView()) * static_cast<illGraphics::VolumeLight*>(light)->m_vector));
                        }
                        else {
                            glUniform3fv(getProgramUniformLocation(prog, "lightDirection"), 1,
                                glm::value_ptr(glm::mat3(camera.getModelView()) * static_cast<illGraphics::VolumeLight*>(light)->m_vector));
                        }                
                                
                        //bind VBO
                        {
                            GLuint buffer = *((GLuint *) m_box.getMeshBackendData() + 0);
                            glBindBuffer(GL_ARRAY_BUFFER, buffer);
                        }

                        //bind IBO
                        {
                            GLuint buffer = *((GLuint *) m_box.getMeshBackendData() + 1);
                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
                        }

                        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, (GLsizei) m_box.getMeshFrontentData()->getVertexSize(), (char *)NULL + m_box.getMeshFrontentData()->getPositionOffset());

                        //transform the planes into eye space
                        {
                            glm::vec4 eyePlanes[illGraphics::MAX_LIGHT_VOLUME_PLANES];

                            for(size_t plane = 0; plane < illGraphics::MAX_LIGHT_VOLUME_PLANES; plane++) {
                                Plane<> eyePlane = static_cast<illGraphics::VolumeLight*>(light)->m_planes[plane].transform(/*glm::transpose(glm::inverse(*/camera.getModelView()/*))*/);
                                eyePlanes[plane] = glm::vec4(eyePlane.m_normal, eyePlane.m_distance);
                            }

                            glUniform4fv(getProgramUniformLocation(prog, "attenuationPlanes"), 12, glm::value_ptr(eyePlanes[0]));
                        }

                        glUniform1fv(getProgramUniformLocation(prog, "attenuationStarts"), 12, static_cast<illGraphics::VolumeLight*>(light)->m_planeFalloff);
                    }
                }
                               
                switch(lightType) {
                case illGraphics::LightBase::Type::POINT:
                case illGraphics::LightBase::Type::POINT_NOSPECULAR:
                case illGraphics::LightBase::Type::SPOT:
                case illGraphics::LightBase::Type::SPOT_NOSPECULAR:
                    glUniform3fv(getProgramUniformLocation(prog, "lightPosition"), 
                        1, glm::value_ptr(getTransformPosition(camera.getModelView() * node->getTransform())));
                        
                    glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 
                        1, false, glm::value_ptr(glm::scale(camera.getModelViewProjection() * node->getTransform(), volumeScale)));
                
                    glUniformMatrix4fv(getProgramUniformLocation(prog, "modelView"), 
                        1, false, glm::value_ptr(glm::scale(camera.getModelView() * node->getTransform(), volumeScale)));

                    break;

                case illGraphics::LightBase::Type::POINT_VOLUME:
                case illGraphics::LightBase::Type::POINT_VOLUME_NOSPECULAR:
                case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME:
                case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME_NOSPECULAR:

                    {
                        glm::mat4 centerTransform = glm::translate(node->getWorldBoundingVolume().getCenter());

                        glUniformMatrix4fv(getProgramUniformLocation(prog, "modelViewProjection"), 
                            1, false, glm::value_ptr(glm::scale(camera.getModelViewProjection() * centerTransform, node->getWorldBoundingVolume().getDimensions())));
                
                        glUniformMatrix4fv(getProgramUniformLocation(prog, "modelView"), 
                            1, false, glm::value_ptr(glm::scale(camera.getModelView() * centerTransform, node->getWorldBoundingVolume().getDimensions())));
                    }

                    break;
                }
                
                //render stencil prepass

                GLuint query = 0;

                if(m_stencilLightingPass) {
                    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                    glGenQueries(1, &query);
                    glBeginQuery(/*GL_SAMPLES_PASSED*/GL_ANY_SAMPLES_PASSED, query);

                    if(node->getOcclusionCull()) {
                        m_nodeQueries.emplace_back();
    
                        m_nodeQueries.back().m_query = query;
                        m_nodeQueries.back().m_node = node;
                        m_nodeQueries.back().m_viewport = viewport;
                    }

                    glEnable(GL_DEPTH_TEST);
                    glDisable(GL_CULL_FACE);
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    
                    glStencilFunc(GL_ALWAYS, 0, 0x00);

                    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
                    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

                    glUniform1i(getProgramUniformLocation(prog, "noLighting"), 1);

#ifdef VERIFY_RENDER_STATE
                    /**
                    Gbuffer FBO bound
					MRT for specular and diffuse accumulation
					
					viewport dimensions set
					
					blend enabled
					blend func additive
					
					depth test enabled
					depth func lequal
					
					stencil test enabled
					stencil func Always, 0, 0x00
					stencil op backFacing: keep, increment wrap, keep
					stencil op frontFacing: keep, decrement wrap, keep
								
					cullface off
					
					depthmask false
					colormask false
					
					shader prog: light volume rendering
										
					mesh VBO bound
					mesh IBO bound
					
					position data set
                    */
                    {
                        GLboolean val;
                        glGetBooleanv(GL_BLEND, &val);
                        assert(val == GL_TRUE);
                    }

                    //TODO: verify blend func

                    {
                        GLboolean val;
                        glGetBooleanv(GL_DEPTH_TEST, &val);
                        assert(val == GL_TRUE);
                    }

                    {
                        GLint val;
                        glGetIntegerv(GL_DEPTH_FUNC, &val);
                        assert(val == GL_LEQUAL);
                    }

                    {
                        GLboolean val;
                        glGetBooleanv(GL_STENCIL_TEST, &val);
                        assert(val == GL_TRUE);
                    }

                    //TODO: verify stencil ops

                    {
                        GLboolean val;
                        glGetBooleanv(GL_CULL_FACE, &val);
                        assert(val == GL_FALSE);
                    }

                    {
                        GLboolean val;
                        glGetBooleanv(GL_DEPTH_WRITEMASK, &val);
                        assert(val == GL_FALSE);
                    }

                    {
                        GLboolean val[4];
                        glGetBooleanv(GL_COLOR_WRITEMASK, val);
                        assert(val[0] == GL_FALSE && val[1] == GL_FALSE && val[2] == GL_FALSE && val[3] == GL_FALSE);
                    }
#endif
                    glDrawRangeElements(GL_TRIANGLES, 0, m_box.getMeshFrontentData()->getNumInd(), m_box.getMeshFrontentData()->getNumInd(), GL_UNSIGNED_SHORT, (char *)NULL);

                    glEndQuery(/*GL_SAMPLES_PASSED*/GL_ANY_SAMPLES_PASSED);
                }

                switch(lightType) {
                case illGraphics::LightBase::Type::POINT:
                case illGraphics::LightBase::Type::POINT_NOSPECULAR:
                    if(camera.getViewFrustum().m_far.distance(getTransformPosition(node->getTransform())) 
                            < static_cast<illGraphics::PointLight*>(light)->m_attenuationEnd) {
                        glCullFace(GL_BACK);
                    }
                    else {
                        glCullFace(GL_FRONT);
                    }
                    break;

                case illGraphics::LightBase::Type::SPOT:
                case illGraphics::LightBase::Type::SPOT_NOSPECULAR:                        
                    if(camera.getViewFrustum().m_far.distance(getTransformPosition(node->getTransform())) 
                            < static_cast<illGraphics::SpotLight*>(light)->m_attenuationEnd) {
                        glCullFace(GL_BACK);
                    }
                    else {
                        glCullFace(GL_FRONT);
                    }
                    break;

                case illGraphics::LightBase::Type::POINT_VOLUME:
                case illGraphics::LightBase::Type::POINT_VOLUME_NOSPECULAR:
                case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME:
                case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME_NOSPECULAR:
                    if(camera.getViewFrustum().m_far.distance(getTransformPosition(node->getTransform())) 
                            < static_cast<illGraphics::SpotLight*>(light)->m_attenuationEnd) {
                        glCullFace(GL_BACK);
                    }
                    else {
                        glCullFace(GL_FRONT);
                    }
                    break;
                }

                //TODO: conditional render
                glBeginConditionalRender(query, GL_ANY_SAMPLES_PASSED/*_CONSERVATIVE*/);

                //render light
                glDisable(GL_DEPTH_TEST);
                glEnable(GL_CULL_FACE);
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

                if(m_stencilLightingPass) {
                    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
                    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                }

                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                glUniform1i(getProgramUniformLocation(prog, "noLighting"), 0);

#ifdef VERIFY_RENDER_STATE
                /**
                Gbuffer FBO bound
				MRT for specular and diffuse accumulation
					
				viewport dimensions set
					
				blend enabled
				blend func additive
					
				depth test disabled
					
				stencil test enabled
				stencil func Not Equal, 0, 0xFF
				stencil op Replace, Replace, Replace (A clever way to clear the stencil buffer)
									
				cullface back/front depending on conditions
				cullface on
					
				depthmask false
				colormask true
					
				shader prog: light volume rendering
										
				mesh VBO bound
				mesh IBO bound
					
				position data set
                */
                {
                    GLboolean val;
                    glGetBooleanv(GL_BLEND, &val);
                    assert(val == GL_TRUE);
                }

                //TODO: verify blend func

                {
                    GLboolean val;
                    glGetBooleanv(GL_DEPTH_TEST, &val);
                    assert(val == GL_FALSE);
                }
                
                {
                    GLboolean val;
                    glGetBooleanv(GL_STENCIL_TEST, &val);
                    assert(val == GL_TRUE);
                }

                //TODO: verify stencil ops

                {
                    GLboolean val;
                    glGetBooleanv(GL_CULL_FACE, &val);
                    assert(val == GL_TRUE);
                }

                {
                    GLboolean val;
                    glGetBooleanv(GL_DEPTH_WRITEMASK, &val);
                    assert(val == GL_FALSE);
                }

                {
                    GLboolean val[4];
                    glGetBooleanv(GL_COLOR_WRITEMASK, val);
                    assert(val[0] == GL_TRUE && val[1] == GL_TRUE && val[2] == GL_TRUE && val[3] == GL_TRUE);
                }
#endif
                glDrawRangeElements(GL_TRIANGLES, 0, m_box.getMeshFrontentData()->getNumInd(), m_box.getMeshFrontentData()->getNumInd(), GL_UNSIGNED_SHORT, (char *)NULL);

                glEndConditionalRender();

                if(!node->getOcclusionCull()) {
                    glDeleteQueries(1, &query);
                }
            }
        }

        glDisableVertexAttribArray(posAttrib);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisable(GL_STENCIL_TEST);

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

    glDisable(GL_BLEND);

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

    glEnable(GL_BLEND);

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

void renderDebugSpotLight(const glm::vec3& position, glm::vec3& direction, const illGraphics::SpotLight& light) {
    renderDebugLightPos(position);
    
    glm::vec3 endPos = direction * light.m_attenuationEnd;
    glm::mediump_float coneDir = glm::degrees(glm::acos(light.m_coneEnd));
    
    glm::vec3 pt0a = glm::mat3(glm::rotate(coneDir, glm::vec3(1.0f, 0.0f, 0.0f))) * endPos + position;
    glm::vec3 pt0b = glm::mat3(glm::rotate(-coneDir, glm::vec3(1.0f, 0.0f, 0.0f))) * endPos + position;

    glm::vec3 pt1a = glm::mat3(glm::rotate(coneDir, glm::vec3(0.0f, 1.0f, 0.0f))) * endPos + position;
    glm::vec3 pt1b = glm::mat3(glm::rotate(-coneDir, glm::vec3(0.0f, 1.0f, 0.0f))) * endPos + position;

    glm::vec3 pt2a = glm::mat3(glm::rotate(coneDir, glm::vec3(0.0f, 0.0f, 1.0f))) * endPos + position;
    glm::vec3 pt2b = glm::mat3(glm::rotate(-coneDir, glm::vec3(0.0f, 0.0f, 1.0f))) * endPos + position;

    endPos += position;

    glBegin(GL_LINES);

    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 1.0f);
    glVertex3fv(glm::value_ptr(position));
    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.1f);
    glVertex3fv(glm::value_ptr(endPos));

    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.5f);
    glVertex3fv(glm::value_ptr(position));
    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.1f);
    glVertex3fv(glm::value_ptr(pt0a));

    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.5f);
    glVertex3fv(glm::value_ptr(position));
    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.1f);
    glVertex3fv(glm::value_ptr(pt0b));

    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.5f);
    glVertex3fv(glm::value_ptr(position));
    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.1f);
    glVertex3fv(glm::value_ptr(pt1a));

    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.5f);
    glVertex3fv(glm::value_ptr(position));
    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.1f);
    glVertex3fv(glm::value_ptr(pt1b));

    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.5f);
    glVertex3fv(glm::value_ptr(position));
    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.1f);
    glVertex3fv(glm::value_ptr(pt2a));

    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.5f);
    glVertex3fv(glm::value_ptr(position));
    glColor4f(light.m_color.x, light.m_color.y, light.m_color.z, 0.1f);
    glVertex3fv(glm::value_ptr(pt2b));

    glEnd();
}

void renderVolumePlaneDebug(const Plane<>& plane, const glm::vec4& color) {
    glm::vec3 x;
    glm::vec3 y;
    glm::vec3 z(0.0f, 0.0f, 1.0f);

    if(eqVec(plane.m_normal, z)) {
        x = glm::vec3(1.0f, 0.0f, 0.0f);
        y = glm::vec3(0.0f, 1.0f, 0.0f);
    }
    else {
        x = glm::cross(plane.m_normal, z);
        y = glm::cross(plane.m_normal, x);
    }

    glm::vec3 P0 = -plane.m_normal * plane.m_distance;  //arbitrary point
    glm::vec3 farX = x * 10000.0f;
    glm::vec3 farY = y * 10000.0f;
    glm::vec3 P1 = P0 - farX - farY;
    glm::vec3 P2 = P0 + farX - farY;
    glm::vec3 P3 = P0 + farX + farY;
    glm::vec3 P4 = P0 - farX + farY;

    glm::vec3 PN = P0 + plane.m_normal * 20.0f;

    glBegin(GL_QUADS);
        glColor4fv(glm::value_ptr(color));
        glVertex3fv(glm::value_ptr(P1));
        glVertex3fv(glm::value_ptr(P2));
        glVertex3fv(glm::value_ptr(P3));
        glVertex3fv(glm::value_ptr(P4));
    glEnd();

    glBegin(GL_LINES);
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        glVertex3fv(glm::value_ptr(P0));
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        glVertex3fv(glm::value_ptr(PN));
    glEnd();
}

void renderDebugVolumeLight(const illGraphics::VolumeLight& light) {
    for(size_t plane = 0; plane < illGraphics::MAX_LIGHT_VOLUME_PLANES; plane++) {
        renderVolumePlaneDebug(light.m_planes[plane], glm::vec4(light.m_color, 0.01f));
    }
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
                auto node = *nodeIter;

                switch(lightType) {
                case illGraphics::LightBase::Type::POINT:
                case illGraphics::LightBase::Type::POINT_NOSPECULAR:
                    renderDebugPointLight(getTransformPosition(node->getTransform()), *static_cast<illGraphics::PointLight *>(light));
                    break;

                case illGraphics::LightBase::Type::SPOT:
                case illGraphics::LightBase::Type::SPOT_NOSPECULAR:
                    renderDebugSpotLight(getTransformPosition(node->getTransform()), glm::mat3(node->getTransform()) * glm::vec3(0.0f, 0.0f, -1.0f), *static_cast<illGraphics::SpotLight *>(light));
                    break;

                case illGraphics::LightBase::Type::POINT_VOLUME:
                case illGraphics::LightBase::Type::POINT_VOLUME_NOSPECULAR:
                case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME:
                case illGraphics::LightBase::Type::DIRECTIONAL_VOLUME_NOSPECULAR:
                    renderDebugVolumeLight(*static_cast<illGraphics::VolumeLight *>(light));
                    break;
                }
            }
        }
    }
}

void renderNodeBounds(const illRendererCommon::GraphicsNode * node) {
    glColor4f(1.0f, 0.0f, 1.0f, 0.4f);

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

    //render the origin
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(node->getTransform()));

    //debug draw the axes
    glBegin(GL_LINES);
    //x Red
        glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 0.0f);

    //y Green
        glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);

    //z Blue
        glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 1.0f);
    glEnd();

    glPopMatrix();
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
                    const auto& meshInfo = *nodeIter;

                    if(m_debugOcclusion) {
                        glMatrixMode(GL_PROJECTION);
                        glLoadIdentity();

                        glMultMatrixf(glm::value_ptr(m_occlusionCamera->getProjection()));

                        glMatrixMode(GL_MODELVIEW);
                        glLoadIdentity();

                        glMultMatrixf(glm::value_ptr(m_occlusionCamera->getModelView()));

                        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y,
                            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
                        
                        glEnable(GL_DEPTH_TEST);

                        renderNodeBounds(meshInfo.m_meshInfo.m_node);

                        glDisable(GL_DEPTH_TEST);
                        
                        glMatrixMode(GL_PROJECTION);
                        glLoadIdentity();

                        glMultMatrixf(glm::value_ptr(camera.getProjection()));

                        glMatrixMode(GL_MODELVIEW);
                        glLoadIdentity();

                        glMultMatrixf(glm::value_ptr(camera.getModelView()));

                        glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y + camera.getViewportDimensions().y / 2,
                            camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
                    }

                    renderNodeBounds(meshInfo.m_meshInfo.m_node);
                }
            }
        }
    }

    for(auto lightTypeIter = renderQueues.m_lights.begin(); lightTypeIter != renderQueues.m_lights.end(); lightTypeIter++) {        
        auto& lights = lightTypeIter->second;

        for(auto lightIter = lights.begin(); lightIter != lights.end(); lightIter++) {            
            auto& lightNodes = lightIter->second;

            for(auto nodeIter = lightNodes.begin(); nodeIter != lightNodes.end(); nodeIter++) {
                const auto& node = *nodeIter;

                if(m_debugOcclusion) {
                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();

                    glMultMatrixf(glm::value_ptr(m_occlusionCamera->getProjection()));

                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();

                    glMultMatrixf(glm::value_ptr(m_occlusionCamera->getModelView()));

                    glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y,
                        camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
                        
                    glEnable(GL_DEPTH_TEST);

                    renderNodeBounds(node);

                    glDisable(GL_DEPTH_TEST);
                        
                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();

                    glMultMatrixf(glm::value_ptr(camera.getProjection()));

                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();

                    glMultMatrixf(glm::value_ptr(camera.getModelView()));

                    glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y + camera.getViewportDimensions().y / 2,
                        camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
                }

                renderNodeBounds(node);
            }
        }
    }
}

void DeferredShadingBackendGl3_3::render(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera, size_t viewport,
        const GridVolume3D<>* debugGridVolume, MeshEdgeList<>* debugFrustum,
        const std::unordered_map<size_t, Array<uint64_t>>* debugLastViewedFrames, uint64_t debugFrameCounter,
        MultiConvexMeshIterator<> * debugMeshIter, std::list<glm::uvec3>* debugTraversals) {
    //enable depth mask
    glDepthMask(GL_TRUE);

    //enable color mask
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    //enable depth func equal
    glDepthFunc(GL_LEQUAL);
    
    //reenable face culling
    glEnable(GL_CULL_FACE);

    renderGbuffer(renderQueues, camera);

    //enable depth mask
    glDepthMask(GL_FALSE);

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
                 
        renderLights(renderQueues, camera, viewport);

        //TODO: forward rendering of blended stuff
        //TODO: post processing

        if(m_debugLights) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            renderDebugLights(renderQueues, camera);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        }

        if(m_debugBounds) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            renderDebugBounds(renderQueues, camera);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        }

        //draw the debug stuff
        if(m_debugOcclusion) {
            glUseProgram(0);
            glEnable(GL_BLEND);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            glActiveTexture(GL_TEXTURE1);
            glDisable(GL_TEXTURE_2D);

            glActiveTexture(GL_TEXTURE2);
            glDisable(GL_TEXTURE_2D);

            glActiveTexture(GL_TEXTURE3);
            glDisable(GL_TEXTURE_2D);

            glActiveTexture(GL_TEXTURE0);
            glEnable(GL_TEXTURE_2D);

            glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y,
                camera.getViewportDimensions().x, camera.getViewportDimensions().y / 2);
                        
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            glMultMatrixf(glm::value_ptr(m_occlusionCamera->getProjection()));

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glMultMatrixf(glm::value_ptr(m_occlusionCamera->getModelView()));

            //renderSceneDebug(*debugGridVolume);

            if(debugMeshIter) {
                glPointSize(5.0f);
                glColor4f(1.0f, 0.0, 1.0f, 0.3f);
                glBegin(GL_POINTS);

                for(auto travIter = debugTraversals->begin(); travIter != debugTraversals->end(); travIter++) {
                    glVertex3fv(glm::value_ptr(vec3cast<unsigned int, float>(*travIter) * debugGridVolume->getCellDimensions() + debugGridVolume->getCellDimensions() * 0.5f));
                }

                glEnd();
            }
                
            renderMeshEdgeListDebug(*debugFrustum);

            //draw the visibility status of a cell
            if(!debugFrustum->m_edges.empty()) {
                ConvexMeshIterator<> meshIter = debugGridVolume->meshIteratorForMesh(debugFrustum);

                while(!meshIter.atEnd()) {
                    unsigned int currentCell = debugGridVolume->indexForCell(meshIter.getCurrentPosition());

                    bool visible = DeferredShadingBackend::decodeVisible(debugLastViewedFrames->at(viewport)[currentCell]);
                    uint64_t lastQueryFrame = DeferredShadingBackend::codeFrame(debugLastViewedFrames->at(viewport)[currentCell]);

                    Box<> cellBox(debugGridVolume->getCellDimensions() * 0.01f, debugGridVolume->getCellDimensions() - debugGridVolume->getCellDimensions() * 0.01f);
                    cellBox += vec3cast<unsigned int, glm::mediump_float>(meshIter.getCurrentPosition()) * debugGridVolume->getCellDimensions();
                    
                    if(visible && lastQueryFrame >= debugFrameCounter) {
                        glColor4f(0.0f, 1.0f, 0.0f, 0.05f);
                    }
                    else {
                        glColor4f(1.0f, 0.0f, 0.0f, 0.05f);
                    }

                    glBegin(GL_LINE_LOOP);
                    glVertex3f(cellBox.m_min.x, cellBox.m_min.y, cellBox.m_min.z);
                    glVertex3f(cellBox.m_max.x, cellBox.m_min.y, cellBox.m_min.z);
                    glVertex3f(cellBox.m_max.x, cellBox.m_max.y, cellBox.m_min.z);
                    glVertex3f(cellBox.m_min.x, cellBox.m_max.y, cellBox.m_min.z);
                    glEnd();

                    glBegin(GL_LINE_LOOP);
                    glVertex3f(cellBox.m_min.x, cellBox.m_min.y, cellBox.m_max.z);
                    glVertex3f(cellBox.m_max.x, cellBox.m_min.y, cellBox.m_max.z);
                    glVertex3f(cellBox.m_max.x, cellBox.m_max.y, cellBox.m_max.z);
                    glVertex3f(cellBox.m_min.x, cellBox.m_max.y, cellBox.m_max.z);
                    glEnd();

                    glBegin(GL_LINES);
                    glVertex3f(cellBox.m_min.x, cellBox.m_min.y, cellBox.m_min.z);
                    glVertex3f(cellBox.m_min.x, cellBox.m_min.y, cellBox.m_max.z);

                    glVertex3f(cellBox.m_max.x, cellBox.m_min.y, cellBox.m_min.z);
                    glVertex3f(cellBox.m_max.x, cellBox.m_min.y, cellBox.m_max.z);

                    glVertex3f(cellBox.m_max.x, cellBox.m_max.y, cellBox.m_min.z);
                    glVertex3f(cellBox.m_max.x, cellBox.m_max.y, cellBox.m_max.z);

                    glVertex3f(cellBox.m_min.x, cellBox.m_max.y, cellBox.m_min.z);
                    glVertex3f(cellBox.m_min.x, cellBox.m_max.y, cellBox.m_max.z);
                    glEnd();

                    meshIter.forward();
                }
            }

            glViewport(camera.getViewportCorner().x, camera.getViewportCorner().y,
                camera.getViewportDimensions().x, camera.getViewportDimensions().y);
        }

        if(m_debugMode != DebugMode::DIFFUSE_ACCUMULATION && m_debugMode != DebugMode::SPECULAR_ACCUMULATION) {
            //disable face culling
            glDisable(GL_CULL_FACE);
            
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

        if(m_debugLights) {
            renderDebugLights(renderQueues, camera);
        }

        if(m_debugBounds) {
            renderDebugBounds(renderQueues, camera);
        }
    }

    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

}