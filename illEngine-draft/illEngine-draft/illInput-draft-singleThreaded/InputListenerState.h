#ifndef __INPUT_LISTER_STATE_H__
#define __INPUT_LISTER_STATE_H__

#include "InputListenerBase.h"

namespace Input {

/**
A state is like a range but true/false with no in between.
A button press starts the action, and a release ends it.
You can provide a call back that acts on only the onPress or onRelease to act like a one time action that isn't continuous.
*/
struct InputListenerState : public InputListenerBase {
    struct InputCallback {
        virtual void onChange(bool value) {}
        virtual void onPress() {}
        virtual void onRelease() {}
    };

    inline InputListenerState() {}

    /**
    Constructs a state.
    @param analogThreshold TODO: figure out doxygen see tag to refer to BinaryAction::m_analogThreshold
    */
    inline InputListenerState(InputCallback * inputCallback, float analogThreshold = 0.1f) 
        : m_inputCallback(inputCallback),
        m_analogThreshold(analogThreshold),
        m_isPressed(false)
    {}

    ~InputListenerState() {
        delete m_inputCallback;
    }

    inline void analogInput(float value) {
        if(value >= m_analogThreshold) {
            onBinPress();
        }
        else {
            onBinRelease();
        }
    }

    inline void onBinPress() {
        if(!m_isPressed) {
            m_isPressed = true;

            m_inputCallback->onChange(m_isPressed);
            m_inputCallback->onPress();
        }
    }

    inline void onBinRelease() {
        if(m_isPressed) {
            m_isPressed = false;

            m_inputCallback->onChange(m_isPressed);
            m_inputCallback->onRelease();
        }
    }

    InputCallback * m_inputCallback;

    float m_analogThreshold;      ///<If the analog signal value is below this, consider it released, otherwise pressed
    bool m_isPressed;             ///<If the action is currently considered pressed or released
};

}

#endif