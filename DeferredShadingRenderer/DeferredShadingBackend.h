#ifndef ILL_DEFERRED_SHADING_BACKEND_H_
#define ILL_DEFERRED_SHADING_BACKEND_H_

#include "RendererCommon/RendererBackend.h"
#include "RendererCommon/serial/RenderQueues.h"

namespace illDeferredShadingRenderer {

class DeferredShadingBackend : public illRendererCommon::RendererBackend {
public:
    DeferredShadingBackend(illGraphics::GraphicsBackend * backend)
        : RendererBackend(backend)
    {}

    /**
    Call this before rendering a frame.
    */
    virtual void setup() = 0;

    /**
    Retreives the cell queries for a view port.
    */
    virtual void retreiveCellQueries(size_t viewPort) = 0;

    /**
    Do a depth pass of objects currently in the depth pass queue in the passed in render queues.
    */
    virtual void depthPass(illRendererCommon::RenderQueues& renderQueues) = 0;

    /**
    Now the scene is ready to be deferred shaded.
    */
    virtual void render(illRendererCommon::RenderQueues& renderQueues) = 0;
};

}

#endif