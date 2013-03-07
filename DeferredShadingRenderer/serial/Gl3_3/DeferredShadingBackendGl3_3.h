#ifndef ILL_DEFERRED_SHADING_BACKEND_GL3_3_H_
#define ILL_DEFERRED_SHADING_BACKEND_GL3_3_H_

#include "DeferredShadingRenderer/DeferredShadingBackend.h"
#include "GlCommon/serial/GlBackend.h"

#include "Graphics/serial/Material/ShaderProgram.h"

namespace illDeferredShadingRenderer {

class DeferredShadingBackendGl3_3 : public DeferredShadingBackend {
public:
    DeferredShadingBackendGl3_3(GlCommon::GlBackend * glBackend)
        : DeferredShadingBackend(glBackend)
    {}

    void initialize(const glm::uvec2 screenResolution);
    void uninitialize();

    virtual void setup();
    virtual void retreiveCellQueries(size_t viewPort);
    virtual void depthPass(illRendererCommon::RenderQueues& renderQueues);
    virtual void render(illRendererCommon::RenderQueues& renderQueues);

private:
    illGraphics::ShaderProgram m_deferredPointLightProgram;
    illGraphics::ShaderProgram m_deferredSpotLightProgram;
};

}

#endif