#ifndef ILL_WINDOW_H__
#define ILL_WINDOW_H__

#include <set>
#include <glm/glm.hpp>

namespace illInput {
class InputManager;
}

namespace illGraphics {

class Renderer;

const float ASPECT_4_3 = 4.0f / 3.0f;
const float ASPECT_16_9 = 16.0f / 9.0f;
const float ASPECT_16_10 = 16.0f / 10.0f;

const int DEFAULT_WIDTH = 1900;         //TODO: these should be 640x480x16 4:3 by default, once configurations are loadable this will be fixed
const int DEFAULT_HEIGHT = 1000;
const int DEFAULT_BPP = 32;
const bool DEFAULT_FULL = false;
const float DEFAULT_ASPECT_RATIO = 0.0f;

class Window {
public:
    enum State {
        WIN_UNINITIALIZED,      ///<The window isn't ready for anything and must be initialized
        WIN_INITIALIZING,       ///<The window is currently initializing either for the first time or after a video mode change
        WIN_RESIZING,           ///<The window is currently resizing in windowed mode so a full restart isn't needed
        WIN_FULL,               ///<The window is fully ready for use in full screen mode
        WIN_WINDOWED            ///<The window is fully ready for use in windowed mode
    };

    Window()
        : m_state(WIN_UNINITIALIZED),

        m_screenWidth(DEFAULT_WIDTH),
        m_screenHeight(DEFAULT_HEIGHT),
        m_screenBPP(DEFAULT_BPP),
        m_fullScreen(DEFAULT_FULL),
        m_aspectRatioSetting(DEFAULT_ASPECT_RATIO)
    {}

    /**
    Initializes a window.  This reinitializes all associated renderers and input managers.
    */
    virtual void initialize() = 0;

    /**
    Completely uninitializes a window.  This shuts down all associated renderers and input managers.
    */
    virtual void uninitialize() = 0;

    /**
    Call this when changing resolution without necessarily calling initialize().
    On Windows, resizing a window wouldn't require an initialize call.
    If in full screen or switching to full screen or out of fullscreen, any screen changes require a full reinitializing.
    */
    virtual void resize() = 0;

    /**
    Polls the window input events
    */
    virtual void pollEvents() = 0;

    /**
    Called at the beginning of a frame
    */
    virtual void beginFrame() = 0;

    /**
    Called at the end of a frame 
    */
    virtual void endFrame() = 0;

    /**
    Returns the current state of the window.
    */
    inline State getState() const {
        return m_state;
    }

    //TODO: don't bother with this stuff until I figure it out on other platforms

    ///**
    //Adds a renderer to the window to be notified of various things.
    //If the window is currently initialized already, it's up to you to initialize the renderer first.
    //*/
    //inline void addRenderer(Renderer * renderer) {
    //    m_renderers.insert(renderer);
    //}
    //
    ///**
    //Removes a renderer from the window to be notified of various things.
    //It's up to you to uninitialize the renderer after removing it.
    //*/
    //inline void removeRenderer(Renderer * renderer) {
    //    std::set<Renderer *>::iterator iter = m_renderers.find(renderer);

    //    if(iter == m_renderers.end()) {
    //        //TODO: log warning
    //    }
    //    else {
    //        m_renderers.erase(iter);
    //    }
    //}

    ///**
    //Returns whether or not a renderer is added to the window.
    //*/
    //inline bool rendererExists(Renderer * renderer) const {
    //    return m_renderers.find(renderer) != m_renderers.end();
    //}

    ///**
    //Removes all renderers from the window to be notified of various things.
    //It's up to you to uninitialize the renderers after removing them.
    //*/
    //inline void clearRenderers() {
    //    m_renderers.clear();
    //}

    ///**
    //Adds an input manager to the window to be notified of polled events.
    //If the window is currently initialized already, it's up to you to initialize the input manager first.
    //*/
    //inline void addInputManager(Input::InputManager * inputManager) {
    //    m_inputManagers.insert(inputManager);
    //}
    //
    ///**
    //Removes an input manager from the window to be notified of polled events.
    //It's up to you to uninitialize the input manager after removing it.
    //*/
    //inline void removeInputManager(Input::InputManager * inputManager) {
    //    std::set<Input::InputManager *>::iterator iter = m_inputManagers.find(inputManager);

    //    if(iter == m_inputManagers.end()) {
    //        //TODO: log warning
    //    }
    //    else {
    //        m_inputManagers.erase(iter);
    //    }
    //}

    ///**
    //Returns whether or not an input manager is added to the window.
    //*/
    //inline bool inputManagerExists(Input::InputManager * inputManager) const {
    //    return m_inputManagers.find(inputManager) != m_inputManagers.end();
    //}

    ///**
    //Removes all input managers from the window to be notified of polled events.
    //It's up to you to uninitialize the input managers after removing them.
    //*/
    //inline void clearInputManagers() {
    //    m_inputManagers.clear();
    //}

    inline glm::uvec2 getResolution() const {
        return glm::uvec2(m_screenWidth, m_screenHeight);
    }

    inline float getAspectRatio() const {
        return m_aspectRatio;
    }

protected:
    State m_state;

    int m_screenWidth;
    int m_screenHeight;
    int m_screenBPP;
    bool m_fullScreen;
    float m_aspectRatio;
    float m_aspectRatioSetting;

    //Meh for now don't bother with this until I know how this would work on other platforms
    //std::set<Renderer *> m_renderers;
    //std::set<Input::InputManager *> m_inputManagers;
};

}

#endif