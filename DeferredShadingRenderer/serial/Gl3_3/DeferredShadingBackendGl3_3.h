#ifndef ILL_DEFERRED_SHADING_BACKEND_GL3_3_H_
#define ILL_DEFERRED_SHADING_BACKEND_GL3_3_H_

#include "RendererCommon/RendererBackend.h"
#include "GlCommon/serial/GlBackend.h"

namespace illDeferredShadingRenderer {

class DeferredShadingBackendGl3_3 : public illRendererCommon::RendererBackend {
public:
    DeferredShadingBackendGl3_3(GlCommon::GlBackend * glBackend)
        : RendererBackend(glBackend)
    {}

private:
    
};

}

#endif