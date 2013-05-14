#ifndef ILL_STATE_LISTENER_H_
#define ILL_STATE_LISTENER_H_

#include "Input/serial/Listeners/ListenerBase.h"

namespace illInput {

/**
A state is like a range but true/false with no in between.
A button press starts the action, and a release ends it.
You can provide a call back that acts on only the onPress or onRelease to act like a one time action that isn't continuous.
*/
struct StateListener : public ListenerBase {    
    /**
    Constructs a state.
    */
    inline StateListener(float analogThreshold = 0.1f) 
        : m_analogThreshold(analogThreshold),
        m_isPressed(false)
    {}

    virtual ~StateListener() {}

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

            onChange(m_isPressed);
            onPress();
        }
    }

    inline void onBinRelease() {
        if(m_isPressed) {
            m_isPressed = false;

            onChange(m_isPressed);
            onRelease();
        }
    }

    virtual void reset() {
        onBinRelease();
    }

    /**
    The function to override and give custom behavior to.  
    This is called any time a state input changes to either pressed or released.
    */
    virtual void onChange(bool value) {}

    /**
    The function to override and give custom behavior to.
    This is called when some state input enters the pressed down state.
    */
    virtual void onPress() {}

    /**
    The function to override and give custom behavior to.
    This is called when some state input enters the released state.
    */
    virtual void onRelease() {}

    float m_analogThreshold;      ///<If the analog signal value is below this, consider it released, otherwise pressed
    bool m_isPressed;             ///<If the action is currently considered pressed or released
};

}

#endif