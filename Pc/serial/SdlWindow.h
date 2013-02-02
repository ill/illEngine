#ifndef ILL_SDL_WINDOW_H__
#define ILL_SDL_WINDOW_H__

#include <SDL.h>

#include "Graphics/Window.h"

namespace illInput {
class InputManager;
}

namespace illGraphics {
class RendererBackend;
}

namespace SdlPc {

//move these constants later
/*const std::string RENDERER_GL2_1("gl2.1");
const std::string RENDERER_GL3_3("gl3.3");
const std::string RENDERER_GL4_2("gl4.2");
const std::string RENDERER_D3D9_0c("d3d9.0c");
const std::string RENDERER_D3D11_1("d3d11.1");

const std::string DEFAULT_RENDERER(RENDERER_GL3_3);*/

/**
This is the main Video update thread.
It contains code for managing the game's window and renderer.
The window spawns in the main thread but the rendering happens in the thread run by this video update.
*/
class SdlWindow : public illGraphics::Window {
public:
    SdlWindow()
        : illGraphics::Window(),
        m_window(NULL)
    {}

    ~SdlWindow();
   
    void initialize();
    void uninitialize();
    void resize();
    void pollEvents();

    void beginFrame() {}
    void endFrame();

    //TODO: this is done this way for now, but in the future don't do it this way.
    inline void setRenderer(illGraphics::RendererBackend * rendererBackend) {
        m_rendererBackend = rendererBackend;
    }

    inline void setInputManager(illInput::InputManager * inputManager) {
        m_inputManager = inputManager;
    }

private:
    //TODO: for now it's hardcoded to have one of each of these
    illGraphics::RendererBackend * m_rendererBackend;
    illInput::InputManager * m_inputManager;

    SDL_Window * m_window;
    void* m_glContext;
};

}

#endif
