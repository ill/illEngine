#ifndef __CAMERA_CONTROLLER_H__
#define __CAMERA_CONTROLLER_H__

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../../illUtil-draft-singleThreaded/Geometry/geomUtil.h"

#include "../../illInput-draft-singleThreaded/InputContext.h"
#include "../../illInput-draft-singleThreaded/InputListenerState.h"
#include "../../illInput-draft-singleThreaded/InputListenerRange.h"
#include "../../illInput-draft-singleThreaded/InputBinding.h"
#include "../../illPC-draft-singleThreaded/sdlInputEnum.h"

#include "../../illUtil-draft-singleThreaded/Logging/logging.h"

namespace Demo {

struct CameraController {
    CameraController()
        : m_speed(0.0f),
        m_rollSpeed(0.0f),

        m_forward(false),
        m_back(false),
        m_left(false),
        m_right(false),
        m_up(false),
        m_down(false),
        m_rollLeft(false),
        m_rollRight(false),
        m_sprint(false),

        m_lookMode(true),

        m_zoom(1.0f)
    {
        //init listeners
        m_horzLookListener.m_controller = this;
        m_vertLookListener.m_controller = this;

        m_forwardListener.m_state = &m_forward;
        m_backListener.m_state = &m_back;
        m_leftListener.m_state = &m_left;
        m_rightListener.m_state = &m_right;
        m_upListener.m_state = &m_up;
        m_downListener.m_state = &m_down;
        m_rollLeftListener.m_state = &m_rollLeft;
        m_rollRightListener.m_state = &m_rollRight;
        m_sprintListener.m_state = &m_sprint;

        m_lookModeListener.m_controller = this;

        m_zoomInListener.m_zoom = &m_zoom;
        m_zoomOutListener.m_zoom = &m_zoom;
        m_zoomDefaultListener.m_zoom = &m_zoom;

        //init inputs
        m_horzLookInput.m_inputCallback = &m_horzLookListener;
        m_vertLookInput.m_inputCallback = &m_vertLookListener;

        m_forwardInput.m_inputCallback = &m_forwardListener;
        m_backInput.m_inputCallback = &m_backListener;
        m_leftInput.m_inputCallback = &m_leftListener;
        m_rightInput.m_inputCallback = &m_rightListener;
        m_upInput.m_inputCallback = &m_upListener;
        m_downInput.m_inputCallback = &m_downListener;
        m_rollLeftInput.m_inputCallback = &m_rollLeftListener;
        m_rollRightInput.m_inputCallback = &m_rollRightListener;
        m_sprintInput.m_inputCallback = &m_sprintListener;

        m_lookModeInput.m_inputCallback = &m_lookModeListener;

        m_zoomInInput.m_inputCallback = &m_zoomInListener;
        m_zoomOutInput.m_inputCallback = &m_zoomOutListener;
        m_zoomDefaultInput.m_inputCallback = &m_zoomDefaultListener;

        //TODO: this should normally be configured externally
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_MOUSE, Input::AX_X), &m_horzLookInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_MOUSE, Input::AX_Y), &m_vertLookInput);   

        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_w), &m_forwardInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_s), &m_backInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_a), &m_leftInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_d), &m_rightInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_SPACE), &m_upInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_LCTRL), &m_downInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_q), &m_rollLeftInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_e), &m_rollRightInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_LSHIFT), &m_sprintInput);

        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_r), &m_lookModeInput);

        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_MOUSE_WHEEL, Input::AX_Y_POS), &m_zoomInInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_MOUSE_WHEEL, Input::AX_Y_NEG), &m_zoomOutInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_MOUSE_BUTTON, 2), &m_zoomDefaultInput);
    }

    ~CameraController() {
    }

    void update(double seconds) {
        glm::vec3 velocity(0.0f);

        if(m_forward) {
            velocity.z = -m_speed;
        }
        else if(m_back) {
            velocity.z = m_speed;
        }

        if(m_left) {
            velocity.x = -m_speed;
        }
        else if(m_right) {
            velocity.x = m_speed;
        }

        if(m_up) {
            velocity.y = m_speed;
        }
        else if(m_down) {
            velocity.y = -m_speed;
        }      

        float speedMultiplier = m_sprint ? 5.0f : 1.0f;

        //velocity = glm::normalize(velocity);

        velocity *= seconds * speedMultiplier;

        if(m_lookMode) {        //quaternion mode
            if(m_rollLeft) {
                m_transform = glm::rotate(m_transform, (float) seconds * m_rollSpeed * speedMultiplier, glm::vec3(0.0f, 0.0f, 1.0f));
            }
            else if(m_rollRight) {
                m_transform = glm::rotate(m_transform, (float) seconds * -m_rollSpeed * speedMultiplier, glm::vec3(0.0f, 0.0f, 1.0f));
            }

            m_transform = glm::translate(m_transform, velocity);
        }
        else {                  //eueler mode
            if(m_rollLeft) {
                m_eulerAngles.z += (float) seconds * m_rollSpeed * speedMultiplier * 0.1f;

                LOG_DEBUG("\nZ: deg: %f rad: %f", 
                    m_eulerAngles.z, glm::radians(m_eulerAngles.z));
            }
            else if(m_rollRight) {
                m_eulerAngles.z -= (float) seconds * m_rollSpeed * speedMultiplier * 0.1f;

                LOG_DEBUG("\nZ: deg: %f rad: %f",
                    m_eulerAngles.z, glm::radians(m_eulerAngles.z));
            }
            
            //Who the hell decided to make this function take them in this order!
            glm::vec3 position = getTransformPosition(m_transform);
            m_transform = glm::yawPitchRoll(m_eulerAngles.y, m_eulerAngles.x, m_eulerAngles.z);

            /*glm::vec3 direction = mat3ToDirection(glm::mat3(m_transform));
            direction.y = 0.0f;
            direction = glm::normalize(direction);

            m_position.y += velocity.y;
            m_position += velocity * direction;*/

            //TODO: make this not suck

            //vertical
            position.y += velocity.y;

            //forward
            glm::mediump_float rad = glm::radians(m_eulerAngles.z);
            
            position.x += velocity.z * glm::cos(rad);
            position.z += velocity.z * glm::sin(rad);

            //strafe
            /*position.x += velocity.x * glm::cos(glm::radians(m_eulerAngles.y + 90.0f));
            position.z += velocity.x * glm::sin(glm::radians(m_eulerAngles.y + 90.0f));*/

            m_transform = setTransformPosition(m_transform, position);
        }
    }

    glm::vec3 m_eulerAngles;
    glm::mat4 m_transform;
    glm::mediump_float m_zoom;

    Input::InputContext m_inputContext;

    float m_speed;
    float m_rollSpeed;

    bool m_forward;
    bool m_back;
    bool m_left;
    bool m_right;
    bool m_up;
    bool m_down;
    bool m_rollLeft;
    bool m_rollRight;
    bool m_sprint;

    bool m_lookMode;

    Input::InputListenerRange m_horzLookInput;
    Input::InputListenerRange m_vertLookInput;

    Input::InputListenerState m_forwardInput;
    Input::InputListenerState m_backInput;
    Input::InputListenerState m_leftInput;
    Input::InputListenerState m_rightInput;
    Input::InputListenerState m_upInput;
    Input::InputListenerState m_downInput;
    Input::InputListenerState m_rollLeftInput;
    Input::InputListenerState m_rollRightInput;
    Input::InputListenerState m_sprintInput;

    Input::InputListenerState m_lookModeInput;

    Input::InputListenerState m_zoomInInput;
    Input::InputListenerState m_zoomOutInput;
    Input::InputListenerState m_zoomDefaultInput;
    
private:
    struct HorzLook : public Input::InputListenerRange::InputCallback {
        HorzLook() 
            : Input::InputListenerRange::InputCallback()
        {}

        virtual ~HorzLook() {}

        void onChange(float value) {
            if(m_controller->m_lookMode) {
                m_controller->m_transform = glm::rotate(m_controller->m_transform, value, glm::vec3(0.0f, -1.0f, 0.0f));
            }
            else {                  //eueler mode
                m_controller->m_eulerAngles.y -= value * 0.1f;

                LOG_DEBUG("\nY: deg: %f rad: %f", 
                    m_controller->m_eulerAngles.y, glm::radians(m_controller->m_eulerAngles.y));
            }
        }

        CameraController * m_controller;
    };

    struct VertLook : public Input::InputListenerRange::InputCallback {
        VertLook()
            : Input::InputListenerRange::InputCallback()
        {}

        virtual ~VertLook() {}

        void onChange(float value) {
            if(m_controller->m_lookMode) {
                m_controller->m_transform = glm::rotate(m_controller->m_transform, value, glm::vec3(-1.0f, 0.0f, 0.0f));
            }
            else {                  //eueler mode
                m_controller->m_eulerAngles.x += value * 0.1f;

                LOG_DEBUG("\nX: deg: %f rad: %f", 
                    m_controller->m_eulerAngles.x, glm::radians(m_controller->m_eulerAngles.x));
            }
        }

        CameraController * m_controller;
    };

    //TODO: the state and toggle listeners look like they'd be useful everywhere, move them to the Input project?
    struct State : public Input::InputListenerState::InputCallback {
        State()
            : Input::InputListenerState::InputCallback()
        {}

        virtual ~State() {}

        void onChange(bool value) {
            *m_state = value;
        }

        bool* m_state;
    };

    struct LookMode : public Input::InputListenerState::InputCallback {
        LookMode()
            : Input::InputListenerState::InputCallback()
        {}

        virtual ~LookMode() {}

        void onRelease() {
            if(m_controller->m_lookMode) {      //switch from quaternion mode to eueler mode
                m_controller->m_lookMode = false;
                
                m_controller->m_eulerAngles = glm::eulerAngles(glm::toQuat(m_controller->m_transform));
            }
            else {                              //switch from eueler mode to quaternion mode
                m_controller->m_lookMode = true;
                
                glm::vec3 position = getTransformPosition(m_controller->m_transform);

                //Who the hell decided to make this function take them in this order!
                m_controller->m_transform = glm::yawPitchRoll(m_controller->m_eulerAngles.y, m_controller->m_eulerAngles.x, m_controller->m_eulerAngles.z);
                m_controller->m_transform = setTransformPosition(m_controller->m_transform, position);
            }
        }

        CameraController * m_controller;
    };

    struct ZoomIn : public Input::InputListenerState::InputCallback {
        ZoomIn()
            : Input::InputListenerState::InputCallback()
        {}

        virtual ~ZoomIn() {}

        void onRelease() {
            *m_zoom -= 0.05f;

            if(*m_zoom <= 0.0f) {
                *m_zoom = 0.05f;
            }
        }

        glm::mediump_float * m_zoom;
    };

    struct ZoomOut : public Input::InputListenerState::InputCallback {
        ZoomOut()
            : Input::InputListenerState::InputCallback()
        {}

        virtual ~ZoomOut() {}

        void onRelease() {
            *m_zoom += 0.05f;
        }

        glm::mediump_float * m_zoom;
    };

    struct ZoomDefault : public Input::InputListenerState::InputCallback {
        ZoomDefault()
            : Input::InputListenerState::InputCallback()
        {}

        virtual ~ZoomDefault() {}

        void onRelease() {
            *m_zoom = 1.0f;
        }

        glm::mediump_float * m_zoom;
    };
    
    HorzLook m_horzLookListener;
    VertLook m_vertLookListener;

    State m_forwardListener;
    State m_backListener;
    State m_leftListener;
    State m_rightListener;
    State m_upListener;
    State m_downListener;
    State m_rollLeftListener;
    State m_rollRightListener;
    State m_sprintListener;

    LookMode m_lookModeListener;

    ZoomIn m_zoomInListener;
    ZoomOut m_zoomOutListener;
    ZoomDefault m_zoomDefaultListener;
};

}

#endif