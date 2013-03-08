#ifndef ILL_DEFERRED_SHADING_BACKEND_H_
#define ILL_DEFERRED_SHADING_BACKEND_H_

#include "RendererCommon/RendererBackend.h"
#include "RendererCommon/serial/RenderQueues.h"

namespace illGraphics {
class Camera;
}

namespace illDeferredShadingRenderer {

class DeferredShadingBackend : public illRendererCommon::RendererBackend {
public:
    DeferredShadingBackend(illGraphics::GraphicsBackend * backend)
        : RendererBackend(backend),
        m_debugMode(DebugMode::NONE)
    {}

    /**
    Call this before rendering a frame.
    */
    virtual void setupFrame() = 0;

    /**
    Call this before rendering a view.
    */
    virtual void setupViewport(const illGraphics::Camera& camera) = 0;

    /**
    Retreives the cell queries for a view port.
    */
    virtual void retreiveCellQueries(size_t viewPort) = 0;
    
    /**
    Do a depth pass of objects currently in the depth pass queue in the passed in render queues.
    */
    virtual void depthPass(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera) = 0;

    /**
    Now the scene is ready to be deferred shaded.
    */
    virtual void render(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera) = 0;
    
    //different debug modes
    enum class DebugMode {
        NONE,

        LIGHT_POS,

        WIRE,
        SOLID,

        DEPTH,
        NORMAL,
        DIFFUSE,
        SPECULAR,

        DIFFUSE_ACCUMULATION,
        SPECULAR_ACCUMULATION
    };

    DebugMode m_debugMode;
};

}

#endif