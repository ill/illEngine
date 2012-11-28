#ifndef __CAMERA_CONTROLLER_H__
#define __CAMERA_CONTROLLER_H__

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../illInput-draft-singleThreaded/InputContext.h"
#include "../../illInput-draft-singleThreaded/InputListenerState.h"
#include "../../illInput-draft-singleThreaded/InputListenerRange.h"
#include "../../illInput-draft-singleThreaded/InputBinding.h"
#include "../../illPC-draft-singleThreaded/sdlInputEnum.h"

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
        m_sprint(false)
    {
        //TODO: the listeners are leaked right now
        //m_horzLookInput = Input::InputListenerRange(new HorzLook(*this));
        //m_vertLookInput = Input::InputListenerRange(new VertLook(*this));

        m_forwardInput = Input::InputListenerState(new Move(m_forward));
        /*m_backInput = Input::InputListenerState(new Move(m_back));
        m_leftInput = Input::InputListenerState(new Move(m_left));
        m_rightInput = Input::InputListenerState(new Move(m_right));
        m_upInput = Input::InputListenerState(new Move(m_up));
        m_downInput = Input::InputListenerState(new Move(m_down));
        m_rollLeftInput = Input::InputListenerState(new Move(m_rollLeft));
        m_rollRightInput = Input::InputListenerState(new Move(m_rollRight));
        m_sprintInput = Input::InputListenerState(new Move(m_sprint));

        m_zoomInInput = Input::InputListenerState(new ZoomIn(*this));
        m_zoomOutInput = Input::InputListenerState(new ZoomOut(*this));
        m_zoomDefaultInput = Input::InputListenerState(new ZoomDefault(*this));*/

        //TODO: this should normally be configured externally
        //m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_MOUSE, Input::AX_X), &m_horzLookInput);
        //m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_MOUSE, Input::AX_Y), &m_vertLookInput);        
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_w), &m_forwardInput);
        /*m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_s), &m_backInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_a), &m_leftInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_d), &m_rightInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_SPACE), &m_upInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_LCTRL), &m_downInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_q), &m_rollLeftInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_e), &m_rollRightInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_KEYBOARD, SDLK_LSHIFT), &m_sprintInput);

        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_MOUSE_WHEEL, Input::AX_Y_POS), &m_zoomInInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_MOUSE_WHEEL, Input::AX_Y_NEG), &m_zoomOutInput);
        m_inputContext.bindInput(Input::InputBinding(SdlPc::PC_MOUSE_BUTTON, 1), &m_zoomDefaultInput);*/
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

        velocity *= seconds * speedMultiplier;

        if(m_rollLeft) {
            m_transform = glm::rotate(m_transform, (float) seconds * m_rollSpeed * speedMultiplier, glm::vec3(0.0f, 0.0f, 1.0f));
        }
        else if(m_rollRight) {
            m_transform = glm::rotate(m_transform, (float) seconds * -m_rollSpeed * speedMultiplier, glm::vec3(0.0f, 0.0f, 1.0f));
        }

        m_transform = glm::translate(m_transform, velocity);
    }

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

    Input::InputListenerState m_zoomInInput;
    Input::InputListenerState m_zoomOutInput;
    Input::InputListenerState m_zoomDefaultInput;

private:
    struct HorzLook : public Input::InputListenerRange::InputCallback {
        HorzLook(CameraController& controller)
            : Input::InputListenerRange::InputCallback(),
            m_controller(controller)
        {}

        void onChange(float value) {
            m_controller.m_transform = glm::rotate(m_controller.m_transform, value, glm::vec3(0.0f, -1.0f, 0.0f));
        }

        CameraController& m_controller;
    };

    struct VertLook : public Input::InputListenerRange::InputCallback {
        VertLook(CameraController& controller)
            : Input::InputListenerRange::InputCallback(),
            m_controller(controller)
        {}

        void onChange(float value) {
            m_controller.m_transform = glm::rotate(m_controller.m_transform, value, glm::vec3(-1.0f, 0.0f, 0.0f));
        }

        CameraController& m_controller;
    };

    struct Move : public Input::InputListenerState::InputCallback {
        Move(bool& state)
            : Input::InputListenerState::InputCallback(),
            m_state(state)
        {}

        void onChange(bool value) {
            m_state = value;
        }

        bool& m_state;
    };

    struct ZoomIn : public Input::InputListenerState::InputCallback {
        ZoomIn(CameraController& controller)
            : Input::InputListenerState::InputCallback(),
            m_controller(controller)
        {}

        void onRelease() {
            m_controller.m_zoom -= 0.01f;

            if(m_controller.m_zoom <= 0.0f) {
                m_controller.m_zoom = 0.01f;
            }
        }

        CameraController& m_controller;
    };

    struct ZoomOut : public Input::InputListenerState::InputCallback {
        ZoomOut(CameraController& controller)
            : Input::InputListenerState::InputCallback(),
            m_controller(controller)
        {}

        void onRelease() {
            m_controller.m_zoom += 0.01f;
        }

        CameraController& m_controller;
    };

    struct ZoomDefault : public Input::InputListenerState::InputCallback {
        ZoomDefault(CameraController& controller)
            : Input::InputListenerState::InputCallback(),
            m_controller(controller)
        {}

        void onRelease() {
            m_controller.m_zoom = 1.0f;
        }

        CameraController& m_controller;
    };
};

}

#endif