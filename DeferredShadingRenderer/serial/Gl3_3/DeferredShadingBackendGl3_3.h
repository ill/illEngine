#ifndef ILL_DEFERRED_SHADING_BACKEND_GL3_3_H_
#define ILL_DEFERRED_SHADING_BACKEND_GL3_3_H_

#include <unordered_map>
#include <vector>

#include <GL/glew.h>

#include "DeferredShadingRenderer/DeferredShadingBackend.h"
#include "GlCommon/serial/GlBackend.h"

#include "Graphics/serial/Material/ShaderProgram.h"
#include "Graphics/serial/Model/Mesh.h"

namespace illDeferredShadingRenderer {

//TODO: there's quite a bit of code in here that is shareable between different renderer types, I'll probably redesign all of this a bit later

class DeferredShadingBackendGl3_3 : public DeferredShadingBackend {
public:
    DeferredShadingBackendGl3_3(GlCommon::GlBackend * glBackend)
        : DeferredShadingBackend(glBackend),
        m_internalShaderProgramLoader(NULL)
    {}

    virtual void initialize(const glm::uvec2 screenResolution, illGraphics::ShaderProgramManager * shaderProgramManager);
    virtual void uninitialize();

    virtual void setupFrame();
    virtual void setupViewport(const illGraphics::Camera& camera);
    virtual void retreiveCellQueries(std::unordered_map<size_t, Array<uint64_t>>& lastViewedFrames, uint64_t lastFrameCounter, uint64_t successDuration, uint64_t failureDuration);
    virtual void retreiveNodeQueries(uint64_t lastFrameCounter);

    virtual void setupQuery();
    virtual void endQuery();
    virtual void * occlusionQueryCell(const illGraphics::Camera& camera, const glm::vec3& cellCenter, const glm::vec3& cellSize,
        unsigned int cellArrayIndex, size_t viewport);
    virtual void * occlusionQueryNode(const illGraphics::Camera& camera, illRendererCommon::GraphicsNode * node, size_t viewport);
    virtual void depthPass(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera, void * cellOcclusionQuery, size_t viewport);
    virtual void render(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera, size_t viewport);

private:
    //data storing an occlusion query for a cell
    struct CellQuery {
        size_t m_viewport;
        GLuint m_query;
        unsigned int m_cellArrayIndex;
    };

    struct NodeQuery {
        size_t m_viewport;
        GLuint m_query;
        const illRendererCommon::GraphicsNode * m_node;
    };

    void setupGbuffer();

    void renderGbuffer(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera);
    void renderAmbientPass(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera);
    void renderEmissivePass(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera);
    void renderLights(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera, size_t viewport);
    //TODO: all the other render tasks
    //TODO: post processing
    void renderFinish();

    void renderDebugLights(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera);
    void renderDebugBounds(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera);

    GLuint m_gBuffer;

    enum RenderTextureType {
        REN_DEPTH,
        REN_NORMAL,
        REN_DIFFUSE,
        REN_SPECULAR,

        REN_DIFFUSE_ACCUMULATION,
        REN_SPECULAR_ACCUMULATION,

        REN_LAST
    };

    GLuint m_renderTextures[REN_LAST];

    illGraphics::ShaderProgramLoader * m_internalShaderProgramLoader;
    illGraphics::ShaderProgram m_deferredPointLightProgram;
    illGraphics::ShaderProgram m_deferredPointLightNoSpecProgram;
    illGraphics::ShaderProgram m_deferredSpotLightProgram;
    illGraphics::ShaderProgram m_deferredSpotLightNoSpecProgram;
    illGraphics::ShaderProgram m_deferredPointVolumeLightProgram;
    illGraphics::ShaderProgram m_deferredPointVolumeLightNoSpecProgram;
    illGraphics::ShaderProgram m_deferredDirectionVolumeLightProgram;
    illGraphics::ShaderProgram m_deferredDirectionVolumeLightNoSpecProgram;
    RefCountPtr<illGraphics::ShaderProgram> m_volumeRenderProgram;

    //TODO: have these be some kind of utility meshes?
    //I might use geometry shaders later, maybe...
    illGraphics::Mesh m_pointLightVolume;
    illGraphics::Mesh m_spotLightVolume;
    illGraphics::Mesh m_box;
    illGraphics::Mesh m_quad;

    std::vector<CellQuery> m_cellQueries;
    std::vector<NodeQuery> m_nodeQueries;

    //testing
    size_t m_currentQuery;
    GLuint m_queryCacheTest[100000];
};

}

#endif