#include <SDL.h>

#include "../illUtil-draft-singleThreaded/Logging/logging.h"

#include "../illGraphics-draft-singleThreaded/RendererBackend.h"

#include "../illInput-draft-singleThreaded/InputManager.h"
#include "../illInput-draft-singleThreaded/InputBinding.h"
#include "../illInput-draft-singleThreaded/inputAxisEnum.h"
#include "../illInput-draft-singleThreaded/InputListenerBase.h"

#include "SdlWindow.h"
#include "sdlInputEnum.h"

namespace SdlPc {

SdlWindow::~SdlWindow() {
    uninitialize();
}

void SdlWindow::initialize() {
    uninitialize();

    SDL_Init(SDL_INIT_VIDEO);
    //disable key repeat
    SDL_EnableKeyRepeat(0, 0);

    m_state = WIN_INITIALIZING;

    Uint32 sdlFlags = SDL_WINDOW_SHOWN;//| SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_GRABBED;

    //TODO: figure out how to make this all modular and stuff to support arbitrary renderers later, for now this works
    sdlFlags |= SDL_WINDOW_OPENGL;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    if(m_fullScreen) {
        sdlFlags |= SDL_WINDOW_FULLSCREEN;
    }

    m_window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_screenWidth, m_screenHeight, sdlFlags);

    m_aspectRatio = m_aspectRatioSetting == 0.0f ? (float) m_screenWidth / m_screenHeight : m_aspectRatioSetting;

    //TODO:make a call to setVideoMode to set the color and refresh rate too

    //TODO: for now there is only OpenGL so figure out later how to make this all modular and stuff to load different renderers like DirectX (try my hand and Direct3D before I start prematurely designing things)
    m_glContext = SDL_GL_CreateContext(m_window);

    m_rendererBackend->initialize();

    m_state = m_fullScreen ? WIN_FULL : WIN_WINDOWED;
}

void SdlWindow::uninitialize() {
    if(m_state == WIN_INITIALIZING || m_state == WIN_RESIZING) {
        LOG_FATAL_ERROR("VideoThread::uninitialize() in unexpected state.");
    }

    if(m_state == WIN_UNINITIALIZED) {
        return;
    }

    m_rendererBackend->uninitialize();
    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_window);

    SDL_Quit();

    m_state = WIN_UNINITIALIZED;
}

void SdlWindow::resize() {
    //if not windowed or switching to windowed gotta completely just reset
    if(m_state != WIN_WINDOWED || m_fullScreen) {
        initialize();
    }

    m_state = WIN_RESIZING;

    SDL_SetWindowSize(m_window, m_screenWidth, m_screenHeight);

    m_state = WIN_WINDOWED;
}

void SdlWindow::pollEvents () {
    //handle events and input
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        Input::InputBinding inputBinding;

        switch(event.type)
        {
        case SDL_KEYDOWN: {
            inputBinding.m_deviceType = PC_KEYBOARD;
            inputBinding.m_input = event.key.keysym.sym;

            //TODO: I could really use some nice functional programming right about now to avoid this code duplication
            //Figure it out later and put it in the base class, or leave it...  C++ can do functional programming so it should work

            const std::set<int> * players = m_inputManager->getPlayersForDevice(inputBinding.m_deviceType);

            if(players != NULL) {
                for(std::set<int>::const_iterator iter = players->begin(); iter != players->end(); iter++) {
                    Input::InputContextStack * inputs = m_inputManager->getInputContextStack(*iter);

                    if(inputs != NULL) {
                        Input::InputListenerBase * inputListener = inputs->lookupBinding(inputBinding);

                        if(inputListener != NULL) {
                            inputListener->onBinPress();
                        }
                    }
                }
            }
        }

        break;

        case SDL_KEYUP: {
            inputBinding.m_deviceType = PC_KEYBOARD;
            inputBinding.m_input = event.key.keysym.sym;

            //TODO: I could really use some nice functional programming right about now to avoid this code duplication
            //Figure it out later and put it in the base class, or leave it...  C++ can do functional programming so it should work

            const std::set<int> * players = m_inputManager->getPlayersForDevice(inputBinding.m_deviceType);

            if(players != NULL) {
                for(std::set<int>::const_iterator iter = players->begin(); iter != players->end(); iter++) {
                    Input::InputContextStack * inputs = m_inputManager->getInputContextStack(*iter);

                    if(inputs != NULL) {
                        Input::InputListenerBase * inputListener = inputs->lookupBinding(inputBinding);

                        if(inputListener != NULL) {
                            inputListener->onBinRelease();
                        }
                    }
                }
            }
        }

        break;

        case SDL_MOUSEBUTTONDOWN: {
            inputBinding.m_deviceType = PC_MOUSE_BUTTON;
            inputBinding.m_input = event.button.button;

            //TODO: I could really use some nice functional programming right about now to avoid this code duplication
            //Figure it out later and put it in the base class, or leave it...  C++ can do functional programming so it should work

            const std::set<int> * players = m_inputManager->getPlayersForDevice(inputBinding.m_deviceType);

            if(players != NULL) {
                for(std::set<int>::const_iterator iter = players->begin(); iter != players->end(); iter++) {
                    Input::InputContextStack * inputs = m_inputManager->getInputContextStack(*iter);

                    if(inputs != NULL) {
                        Input::InputListenerBase * inputListener = inputs->lookupBinding(inputBinding);

                        if(inputListener != NULL) {
                            inputListener->onBinPress();
                        }
                    }
                }
            }
        }

        break;

        case SDL_MOUSEBUTTONUP: {
            inputBinding.m_deviceType = PC_MOUSE_BUTTON;
            inputBinding.m_input = event.button.button;

            //TODO: I could really use some nice functional programming right about now to avoid this code duplication
            //Figure it out later and put it in the base class, or leave it...  C++ can do functional programming so it should work

            const std::set<int> * players = m_inputManager->getPlayersForDevice(inputBinding.m_deviceType);

            if(players != NULL) {
                for(std::set<int>::const_iterator iter = players->begin(); iter != players->end(); iter++) {
                    Input::InputContextStack * inputs = m_inputManager->getInputContextStack(*iter);

                    if(inputs != NULL) {
                        Input::InputListenerBase * inputListener = inputs->lookupBinding(inputBinding);

                        if(inputListener != NULL) {
                            inputListener->onBinRelease();
                        }
                    }
                }
            }
        }

        break;

        //TODO: I'll need to refactor this so it's more clean and not duplicating code all over the place later...
        case SDL_MOUSEWHEEL: {
            //it seems to send in 120 as the movement distance, so normalize.
            event.wheel.x /= 120;
            event.wheel.y /= 120;

            inputBinding.m_deviceType = PC_MOUSE_WHEEL;            

            //TODO: I could really use some nice functional programming right about now to avoid this code duplication
            //Figure it out later and put it in the base class, or leave it...  C++ can do functional programming so it should work

            const std::set<int> * players = m_inputManager->getPlayersForDevice(inputBinding.m_deviceType);

            if(players != NULL) {
                for(std::set<int>::const_iterator iter = players->begin(); iter != players->end(); iter++) {
                    Input::InputContextStack * inputs = m_inputManager->getInputContextStack(*iter);

                    if(inputs != NULL) {
                        Input::InputListenerBase * inputListener;

                        //x axis
                        inputBinding.m_input = Input::AX_X;
                        inputListener = inputs->lookupBinding(inputBinding);

                        if(inputListener != NULL) {
                            inputListener->analogInput((float) event.wheel.x);
                            inputListener->analogInput(0.0f);
                        }

                        //y axis
                        inputBinding.m_input = Input::AX_Y;
                        inputListener = inputs->lookupBinding(inputBinding);

                        if(inputListener != NULL) {
                            inputListener->analogInput((float) event.wheel.y);
                            inputListener->analogInput(0.0f);
                        }

                        //x positive axis
                        if(event.wheel.x > 0) {
                            inputBinding.m_input = Input::AX_X_POS;
                            inputListener = inputs->lookupBinding(inputBinding);

                            if(inputListener != NULL) {
                                inputListener->analogInput((float) event.wheel.x);
                                inputListener->analogInput(0.0f);
                            }
                        }

                        //x negative axis
                        if(event.wheel.x < 0) {
                            inputBinding.m_input = Input::AX_X_NEG;
                            inputListener = inputs->lookupBinding(inputBinding);

                            if(inputListener != NULL) {
                                inputListener->analogInput((float) -event.wheel.x);
                                inputListener->analogInput(0.0f);
                            }
                        }

                        //y positive axis
                        if(event.wheel.y > 0) {
                            inputBinding.m_input = Input::AX_Y_POS;
                            inputListener = inputs->lookupBinding(inputBinding);

                            if(inputListener != NULL) {
                                inputListener->analogInput((float) event.wheel.y);
                                inputListener->analogInput(0.0f);
                            }
                        }

                        //y negative axis
                        if(event.wheel.y < 0) {
                            inputBinding.m_input = Input::AX_Y_NEG;
                            inputListener = inputs->lookupBinding(inputBinding);

                            if(inputListener != NULL) {
                                inputListener->analogInput((float) -event.wheel.y);
                                inputListener->analogInput(0.0f);
                            }
                        }
                    }
                }
            }
        }
        break;

        case SDL_MOUSEMOTION: {
            inputBinding.m_deviceType = PC_MOUSE;            

            //TODO: I could really use some nice functional programming right about now to avoid this code duplication
            //Figure it out later and put it in the base class, or leave it...  C++ can do functional programming so it should work

            const std::set<int> * players = m_inputManager->getPlayersForDevice(inputBinding.m_deviceType);

            if(players != NULL) {
                for(std::set<int>::const_iterator iter = players->begin(); iter != players->end(); iter++) {
                    Input::InputContextStack * inputs = m_inputManager->getInputContextStack(*iter);

                    if(inputs != NULL) {
                        Input::InputListenerBase * inputListener;

                        //mouse x value
                        inputBinding.m_input = Input::AX_X_VAL;
                        inputListener = inputs->lookupBinding(inputBinding);

                        if(inputListener != NULL) {
                            inputListener->analogInput((float) event.motion.x);
                        }

                        //mouse y value
                        inputBinding.m_input = Input::AX_Y_VAL;
                        inputListener = inputs->lookupBinding(inputBinding);

                        if(inputListener != NULL) {
                            inputListener->analogInput((float) event.motion.y);
                        }

                        //mouse x axis
                        inputBinding.m_input = Input::AX_X;
                        inputListener = inputs->lookupBinding(inputBinding);

                        if(inputListener != NULL) {
                            inputListener->analogInput((float) event.motion.xrel);
                        }

                        //mouse y axis
                        inputBinding.m_input = Input::AX_Y;
                        inputListener = inputs->lookupBinding(inputBinding);

                        if(inputListener != NULL) {
                            inputListener->analogInput((float) event.motion.yrel);
                        }

                        //mouse x positive axis
                        if(event.motion.xrel > 0) {
                            inputBinding.m_input = Input::AX_X_POS;
                            inputListener = inputs->lookupBinding(inputBinding);

                            if(inputListener != NULL) {
                                inputListener->analogInput((float) event.motion.xrel);
                            }
                        }

                        //mouse x negative axis
                        if(event.motion.xrel < 0) {
                            inputBinding.m_input = Input::AX_X_NEG;
                            inputListener = inputs->lookupBinding(inputBinding);

                            if(inputListener != NULL) {
                                inputListener->analogInput((float) -event.motion.xrel);
                            }
                        }

                        //mouse y positive axis
                        if(event.motion.yrel > 0) {
                            inputBinding.m_input = Input::AX_Y_POS;
                            inputListener = inputs->lookupBinding(inputBinding);

                            if(inputListener != NULL) {
                                inputListener->analogInput((float) event.motion.yrel);
                            }
                        }

                        //mouse y negative axis
                        if(event.motion.yrel < 0) {
                            inputBinding.m_input = Input::AX_Y_NEG;
                            inputListener = inputs->lookupBinding(inputBinding);

                            if(inputListener != NULL) {
                                inputListener->analogInput((float) -event.motion.yrel);
                            }
                        }
                    }
                }
            }
        }

        break;

        case SDL_QUIT:

            //Application::applicationThread->stop();
            //TODO: have an on window close and window minimize and all those other buttons listener
            exit(0);

            break;
        }
    }
}

void SdlWindow::endFrame() {
    //TODO: for now this is hardcoded as if it was working with OpenGL
    SDL_GL_SwapWindow(m_window);
}

}
