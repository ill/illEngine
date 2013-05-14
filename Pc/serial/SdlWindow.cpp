#include <SDL.h>

#include "Logging/logging.h"

#include "Graphics/GraphicsBackend.h"

#include "Input/serial/InputManager.h"
#include "Input/serial/InputBinding.h"
#include "Input/serial/inputEnum.h"
#include "Input/serial/Listeners/ListenerBase.h"
#include "Input/serial/Listeners/ValueListener.h"

#include "Console/serial/DeveloperConsole.h"

#include "SdlWindow.h"
#include "sdlInputEnum.h"

namespace SdlPc {

SdlWindow::~SdlWindow() {
    uninitialize();
}

void SdlWindow::initialize() {
    uninitialize();

    SDL_Init(SDL_INIT_VIDEO);
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

    m_graphicsBackend->initialize();

    m_state = m_fullScreen ? WIN_FULL : WIN_WINDOWED;
}

void SdlWindow::uninitialize() {
    if(m_state == WIN_INITIALIZING || m_state == WIN_RESIZING) {
        LOG_FATAL_ERROR("VideoThread::uninitialize() in unexpected state.");
    }

    if(m_state == WIN_UNINITIALIZED) {
        return;
    }

    m_graphicsBackend->uninitialize();
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

void SdlWindow::beginTypingInput(TypingInfo* destination) {
    m_typingDestination = destination;
    SDL_StartTextInput();

    SDL_Rect rect = {
        (int) destination->m_editRectCorner.x, 
        (int) destination->m_editRectCorner.y,
        (int) destination->m_editRectSize.x, 
        (int) destination->m_editRectSize.y
    };

    SDL_SetTextInputRect(&rect);
}

void SdlWindow::endTypingInput() {
    m_typingDestination = NULL;
    SDL_StopTextInput();
}

void SdlWindow::pollEvents () {
    //handle events and input
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        bool handledTypeEvent = m_typingDestination && SDL_HasEvent(SDL_TEXTINPUT);
        illInput::InputBinding inputBinding;

        switch(event.type)
        {
        case SDL_TEXTINPUT:            
            if(m_typingDestination) {
                //insert new text
                size_t typeTextSize = strlen(event.text.text);
                size_t moveTextSize = strlen(m_typingDestination->m_destination + m_typingDestination->m_selectionStart);
                size_t maxPos = m_typingDestination->m_destinationLimit - 1;

                //truncate text being added if it won't fit to the string
                size_t realTypeTextSize = glm::min(typeTextSize, maxPos - m_typingDestination->m_selectionStart);
                
                size_t newPos = m_typingDestination->m_selectionStart + realTypeTextSize;

                //move the text if it'll fit
                if(realTypeTextSize == typeTextSize) {
                    size_t realMoveTextSize = glm::min(maxPos - newPos, moveTextSize);

                    memmove(m_typingDestination->m_destination + newPos, m_typingDestination->m_destination + m_typingDestination->m_selectionStart,
                        realMoveTextSize);

                    //null terminate
                    m_typingDestination->m_destination[newPos + realMoveTextSize] = 0;
                }
                else {
                    m_typingDestination->m_destination[maxPos] = 0;
                }

                memcpy(m_typingDestination->m_destination + m_typingDestination->m_selectionStart, event.text.text, realTypeTextSize);

                m_typingDestination->m_selectionStart += realTypeTextSize;
            }
            break;
        case SDL_TEXTEDITING:
            /*
            Update the composition text.
            Update the cursor position.
            Update the selection length (if any).
            */
            if(m_typingDestination) {
                //I don't know how this event works...
            }

            /*composition = event.edit.text;
            cursor = event.edit.start;
            selection_len = event.edit.length;*/
            break;

        case SDL_KEYDOWN: {
            if(m_typingDestination) {
                //handle the del and backspace keys since type event doesn't seem to
                //this will probably handle multichar inputs wrong

                if(event.key.keysym.sym == SDLK_BACKSPACE) {
                    size_t textSize = strnlen(m_typingDestination->m_destination + m_typingDestination->m_selectionStart, 
                        m_typingDestination->m_destinationLimit - m_typingDestination->m_selectionStart) + 1;

                    memmove(m_typingDestination->m_destination + m_typingDestination->m_selectionStart - 1, 
                        m_typingDestination->m_destination + m_typingDestination->m_selectionStart,
                        textSize);

                    --m_typingDestination->m_selectionStart;

                    handledTypeEvent = true;
                }
                else if(event.key.keysym.sym == SDLK_DELETE) {
                    size_t textSize = strnlen(m_typingDestination->m_destination + m_typingDestination->m_selectionStart + 1, 
                        m_typingDestination->m_destinationLimit - m_typingDestination->m_selectionStart + 1) + 1;

                    memmove(m_typingDestination->m_destination + m_typingDestination->m_selectionStart, 
                        m_typingDestination->m_destination + m_typingDestination->m_selectionStart + 1,
                        textSize);

                    handledTypeEvent = true;
                }
            }
            
            if(!handledTypeEvent) {
                inputBinding.m_deviceType = (int) SdlPc::InputType::PC_KEYBOARD;
                inputBinding.m_input = event.key.keysym.sym;
            
                const std::set<int> * players = m_inputManager->getPlayersForDevice(inputBinding.m_deviceType);

                if(players != NULL) {
                    for(auto playerIter = players->cbegin(); playerIter != players->cend(); playerIter++) {
                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        if(inputs) {
                                            illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                            if(inputListener != NULL) {
                                                inputListener->onBinPress();
                                            }
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
                            }
                        }
                    }
                }
            }
        }

        break;

        case SDL_KEYUP: {
            if(!handledTypeEvent) {
                inputBinding.m_deviceType = (int) SdlPc::InputType::PC_KEYBOARD;
                inputBinding.m_input = event.key.keysym.sym;

                const std::set<int> * players = m_inputManager->getPlayersForDevice(inputBinding.m_deviceType);

                if(players != NULL) {
                    for(auto playerIter = players->cbegin(); playerIter != players->cend(); playerIter++) {
                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->onBinRelease();
                                        }
                                    } break;
                                }
                            }
                        }
                    }
                }
            }
        }

        break;

        case SDL_MOUSEBUTTONDOWN: {
            inputBinding.m_deviceType = (int) SdlPc::InputType::PC_MOUSE_BUTTON;
            inputBinding.m_input = event.button.button;

            const std::set<int> * players = m_inputManager->getPlayersForDevice(inputBinding.m_deviceType);

            if(players != NULL) {
                for(auto playerIter = players->cbegin(); playerIter != players->cend(); playerIter++) {
                    auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                    illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                    if(inputActions) {
                        for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                            switch(actionIter->second) {
                                case illInput::InputManager::ActionType::CONTROL: {
                                    if(inputs) {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->onBinPress();
                                        }
                                    }
                                } break;

                                case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                    if(m_developerConsole) {
                                        m_developerConsole->parseInput(actionIter->first.c_str());
                                    }
                                } break;
                            }
                        }
                    }
                }
            }
        }

        break;

        case SDL_MOUSEBUTTONUP: {
            inputBinding.m_deviceType = (int) SdlPc::InputType::PC_MOUSE_BUTTON;
            inputBinding.m_input = event.button.button;

            const std::set<int> * players = m_inputManager->getPlayersForDevice(inputBinding.m_deviceType);

            if(players != NULL) {
                for(auto playerIter = players->cbegin(); playerIter != players->cend(); playerIter++) {
                    auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                    illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                    if(inputActions && inputs) {
                        for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {                            
                            switch(actionIter->second) {
                                case illInput::InputManager::ActionType::CONTROL: {
                                    illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                    if(inputListener != NULL) {
                                        inputListener->onBinRelease();
                                    }
                                } break;
                            }
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

            inputBinding.m_deviceType = (int) SdlPc::InputType::PC_MOUSE_WHEEL;            

            const std::set<int> * players = m_inputManager->getPlayersForDevice(inputBinding.m_deviceType);

            if(players != NULL) {
                for(auto playerIter = players->cbegin(); playerIter != players->cend(); playerIter++) {
                    {
                        inputBinding.m_input = (int)illInput::Axis::AX_X;

                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->analogInput((float) event.wheel.x);
                                            inputListener->analogInput(0.0f);
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
                            }
                        }
                    }

                    {
                        inputBinding.m_input = (int)illInput::Axis::AX_Y;

                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->analogInput((float) event.wheel.y);
                                            inputListener->analogInput(0.0f);
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
                            }
                        }
                    }

                    if(event.wheel.x > 0) {
                        inputBinding.m_input = (int)illInput::Axis::AX_X_POS;

                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->analogInput((float) event.wheel.x);
                                            inputListener->analogInput(0.0f);
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
                            }
                        }
                    }

                    if(event.wheel.x < 0) {
                        inputBinding.m_input = (int)illInput::Axis::AX_X_NEG;

                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->analogInput((float) -event.wheel.x);
                                            inputListener->analogInput(0.0f);
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
                            }
                        }
                    }

                    if(event.wheel.y > 0) {
                        inputBinding.m_input = (int)illInput::Axis::AX_Y_POS;

                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->analogInput((float) event.wheel.y);
                                            inputListener->analogInput(0.0f);
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
                            }
                        }
                    }

                    if(event.wheel.y < 0) {
                        inputBinding.m_input = (int)illInput::Axis::AX_Y_NEG;

                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->analogInput((float) -event.wheel.y);
                                            inputListener->analogInput(0.0f);
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
                            }
                        }
                    }
                }
            }
        }
        break;

        case SDL_MOUSEMOTION: {
            inputBinding.m_deviceType = (int) SdlPc::InputType::PC_MOUSE;            

            const std::set<int> * players = m_inputManager->getPlayersForDevice(inputBinding.m_deviceType);

            if(players != NULL) {
                for(auto playerIter = players->cbegin(); playerIter != players->cend(); playerIter++) {
                    {
                        inputBinding.m_input = (int)illInput::Axis::AX_VAL;

                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ValueListener * valueListener = inputs->lookupValueBinding(actionIter->first.c_str());

                                        if(valueListener != NULL) {
                                            MousePosition pos(event.motion.x, event.motion.y);

                                            valueListener->onChange(CopiedData(&pos, sizeof(MousePosition)));
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
                            }
                        }
                    }

                    {
                        inputBinding.m_input = (int)illInput::Axis::AX_X;

                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->analogInput((float) event.motion.xrel);
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
                            }
                        }
                    }

                    {
                        inputBinding.m_input = (int)illInput::Axis::AX_Y;

                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->analogInput((float) event.motion.yrel);
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
                            }
                        }
                    }

                    if(event.motion.xrel > 0) {
                        inputBinding.m_input = (int)illInput::Axis::AX_X_POS;

                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->analogInput((float) event.motion.xrel);
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
                            }
                        }
                    }

                    if(event.motion.xrel < 0) {
                        inputBinding.m_input = (int)illInput::Axis::AX_X_NEG;

                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->analogInput((float) -event.motion.xrel);
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
                            }
                        }
                    }

                    if(event.motion.yrel > 0) {
                        inputBinding.m_input = (int)illInput::Axis::AX_Y_POS;

                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->analogInput((float) event.motion.yrel);
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
                            }
                        }
                    }

                    if(event.motion.yrel < 0) {
                        inputBinding.m_input = (int)illInput::Axis::AX_Y_NEG;

                        auto inputActions = m_inputManager->getInputActionBindings(*playerIter, inputBinding);
                        illInput::InputContextStack * inputs = m_inputManager->getInputContextStack(*playerIter);

                        if(inputActions && inputs) {
                            for(auto actionIter = inputActions->cbegin(); actionIter != inputActions->cend(); actionIter++) {
                                switch(actionIter->second) {
                                    case illInput::InputManager::ActionType::CONTROL: {
                                        illInput::ListenerBase * inputListener = inputs->lookupBinding(actionIter->first.c_str());

                                        if(inputListener != NULL) {
                                            inputListener->analogInput((float) -event.motion.yrel);
                                        }
                                    } break;

                                    case illInput::InputManager::ActionType::CONSOLE_COMMAND: {
                                        if(m_developerConsole) {
                                            m_developerConsole->parseInput(actionIter->first.c_str());
                                        }
                                    } break;
                                }
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
