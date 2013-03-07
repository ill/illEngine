#ifndef ILL_RENDERER_BACKEND_H_
#define ILL_RENDERER_BACKEND_H_

namespace illGraphics {
class GraphicsBackend;
}

namespace illRendererCommon {
class RendererBackend {
public:
    enum class State {
        INITIALIZED,
        UNINITIALIZED
    };

    virtual void initialize(const glm::uvec2 screenResolution) = 0;
    virtual void uninitialize() = 0;

    inline State getState() const {
        return m_state;
    }

protected:
    RendererBackend(illGraphics::GraphicsBackend * backend)
        : m_state(State::UNINITIALIZED),
        m_graphicsBackend(backend)
    {}

    State m_state;

    illGraphics::GraphicsBackend * m_graphicsBackend;
};
}

#endif