#ifndef __CAMERA_CONTROLLER_H__
#define __CAMERA_CONTROLLER_H__

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
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
    CameraController();
    ~CameraController() {}

    void update(double seconds);

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
            : Input::InputListenerRange::InputCallback(),
            m_controller(NULL)
        {}

        virtual ~HorzLook() {}

        void onChange(float value) {
            if(m_controller->m_lookMode) {
                m_controller->m_transform = glm::rotate(m_controller->m_transform, value, glm::vec3(0.0f, -1.0f, 0.0f));
            }
            else {                  //eueler mode
                m_controller->m_eulerAngles.y -= value;
            }
        }

        CameraController * m_controller;
    };

    struct VertLook : public Input::InputListenerRange::InputCallback {
        VertLook()
            : Input::InputListenerRange::InputCallback(),
            m_controller(NULL)
        {}

        virtual ~VertLook() {}

        void onChange(float value) {
            if(m_controller->m_lookMode) {
                m_controller->m_transform = glm::rotate(m_controller->m_transform, value, glm::vec3(-1.0f, 0.0f, 0.0f));
            }
            else {                  //eueler mode                
                m_controller->m_eulerAngles.x -= value;

                if(m_controller->m_eulerAngles.x > 90) {
                    m_controller->m_eulerAngles.x = 90;
                }

                if(m_controller->m_eulerAngles.x < -90) {
                    m_controller->m_eulerAngles.x = -90;
                }
            }
        }

        CameraController * m_controller;
    };

    //TODO: the state and toggle listeners look like they'd be useful everywhere, move them to the Input project?
    struct State : public Input::InputListenerState::InputCallback {
        State()
            : Input::InputListenerState::InputCallback(),
            m_state(NULL)
        {}

        virtual ~State() {}

        void onChange(bool value) {
            *m_state = value;
        }

        bool* m_state;
    };

    struct LookMode : public Input::InputListenerState::InputCallback {
        LookMode()
            : Input::InputListenerState::InputCallback(),
            m_controller(NULL)
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
                m_controller->m_transform = glm::yawPitchRoll(glm::radians(m_controller->m_eulerAngles.y), glm::radians(m_controller->m_eulerAngles.x), glm::radians(m_controller->m_eulerAngles.z));
                m_controller->m_transform = setTransformPosition(m_controller->m_transform, position);
            }
        }

        CameraController * m_controller;
    };

    struct ZoomIn : public Input::InputListenerState::InputCallback {
        ZoomIn()
            : Input::InputListenerState::InputCallback(),
            m_zoom(NULL)
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
            : Input::InputListenerState::InputCallback(),
            m_zoom(NULL)
        {}

        virtual ~ZoomOut() {}

        void onRelease() {
            *m_zoom += 0.05f;
        }

        glm::mediump_float * m_zoom;
    };

    struct ZoomDefault : public Input::InputListenerState::InputCallback {
        ZoomDefault()
            : Input::InputListenerState::InputCallback(),
            m_zoom(NULL)
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
