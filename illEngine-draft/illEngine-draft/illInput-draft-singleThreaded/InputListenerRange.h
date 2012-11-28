#ifndef __INPUT_LISTENER_RANGE_H__
#define __INPUT_LISTENER_RANGE_H__

#include "InputListenerBase.h"

namespace Input {

/**
A range usually has some analog input associated with it like joystick position or mouse movement.
It's possible to still map button presses to a range, it just treats the button being down as the full input of 1.0.

The range is either from 0 to 1 if normalized, or 0 to some number.  Mouse motion doesn't make sense to normalize.

If mapping a joystick/mouse motion to a range, you will need 2 ranges per axis if you also allow buttons to map to the input.
One will be for positive values, other for negative values, since the button presses map to either input of 0 or 1.
When you send an analog value to the negative range, negate it before passing it in, and then negate it again when reading it.

If you only ever allow analog inputs and would never allow keys to map to a range, don't bother.  Just send the negative values in.
*/
struct InputListenerRange : public InputListenerBase {
    struct InputCallback {
        virtual void onChange(float value) {}
    };

    inline InputListenerRange() {}

    /**
    Constructs a range input.
    */
    inline InputListenerRange(InputCallback* inputCallback) 
        : m_inputCallback(inputCallback)
    {}

    ~InputListenerRange() {
    }

    inline void analogInput(float value) {
        m_inputCallback->onChange(value);
    }

    inline void onBinPress() {
        analogInput(1.0f);
    }

    inline void onBinRelease() {
        analogInput(0.0f);
    }

    InputCallback * m_inputCallback;
    float m_normalizeFactor;      
};

}

#endif