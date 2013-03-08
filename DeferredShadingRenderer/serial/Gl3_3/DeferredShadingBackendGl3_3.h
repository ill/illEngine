#ifndef ILL_DEFERRED_SHADING_BACKEND_GL3_3_H_
#define ILL_DEFERRED_SHADING_BACKEND_GL3_3_H_

#include <GL/glew.h>

#include "DeferredShadingRenderer/DeferredShadingBackend.h"
#include "GlCommon/serial/GlBackend.h"

#include "Graphics/serial/Material/ShaderProgram.h"

namespace illDeferredShadingRenderer {

class DeferredShadingBackendGl3_3 : public DeferredShadingBackend {
public:
    DeferredShadingBackendGl3_3(GlCommon::GlBackend * glBackend)
        : DeferredShadingBackend(glBackend)
    {}

    virtual void initialize(const glm::uvec2 screenResolution);
    virtual void uninitialize();

    virtual void setupFrame();
    virtual void setupViewport(const illGraphics::Camera& camera);
    virtual void retreiveCellQueries(size_t viewPort);
    virtual void depthPass(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera);
    virtual void render(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera);

private:
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

    illGraphics::ShaderProgram m_deferredPointLightProgram;
    illGraphics::ShaderProgram m_deferredSpotLightProgram;
};

}

#endif