#include "CameraController.h"

namespace Demo {

CameraController::CameraController()
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

void CameraController::update(double seconds) {
    glm::vec3 velocity(0.0f);

    if(m_forward) {
        velocity.z = -1.0f;
    }
    else if(m_back) {
        velocity.z = 1.0f;
    }

    if(m_left) {
        velocity.x = -1.0f;
    }
    else if(m_right) {
        velocity.x = 1.0f;
    }

    if(m_up) {
        velocity.y = 1.0f;
    }
    else if(m_down) {
        velocity.y = -1.0f;
    }      

    float speedMultiplier = m_sprint ? 5.0f : 1.0f;

    velocity = safeNormalize(velocity);

    velocity *= seconds * m_speed * speedMultiplier;

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
            m_eulerAngles.z += (float) seconds * m_rollSpeed * speedMultiplier;
        }
        else if(m_rollRight) {
            m_eulerAngles.z -= (float) seconds * m_rollSpeed * speedMultiplier;
        }
            
        //Who the hell decided to make this function take them in this order!
        glm::vec3 position = getTransformPosition(m_transform);
        glm::mat4 rotation = glm::yawPitchRoll(glm::radians(m_eulerAngles.y), glm::radians(m_eulerAngles.x), glm::radians(m_eulerAngles.z));
            
        //TODO: make this not suck

        //vertical
        position.y += velocity.y;

        //forward
            
        glm::mediump_float rad = glm::radians(m_eulerAngles.y - 90);
            
        position.x += velocity.z * glm::cos(rad);
        position.z -= velocity.z * glm::sin(rad);
            
        //strafe
        rad = glm::radians(m_eulerAngles.y);
            
        position.x += velocity.x * glm::cos(rad);
        position.z -= velocity.x * glm::sin(rad);

        m_transform = glm::translate(position);
        m_transform = m_transform * rotation;
    }
}

}