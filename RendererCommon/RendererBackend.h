#ifndef ILL_RENDERER_BACKEND_H_
#define ILL_RENDERER_BACKEND_H_

namespace illGraphics {
class GraphicsBackend;
}

namespace illRendererCommon {
class RendererBackend {
public:

protected:
    RendererBackend(illGraphics::GraphicsBackend * backend)
        : m_graphicsBackend(backend)
    {}

    illGraphics::GraphicsBackend * m_graphicsBackend;
};
}

#endif