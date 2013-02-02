#ifndef ILL_RANGE_LISTENER_H_
#define ILL_RANGE_LISTENER_H_

#include "Input/serial/Listeners/ListenerBase.h"

namespace illInput {

/**
A range usually has some analog input associated with it like joystick position or mouse movement.
It's possible to still map button presses to a range, it just treats the button being down as the full input of 1.0.

The range is either from 0 to 1 if normalized.

If mapping a joystick/mouse motion to a range, you will need 2 ranges per axis if you also allow buttons to map to the input.
One will be for positive values, other for negative values, since the button presses map to either input of 0 or 1.
When you send an analog value to the negative range, negate it before passing it in, and then negate it again when reading it.

If you only ever allow analog inputs and would never allow keys to map to a range, don't bother.  Just send the negative values in.
*/
struct RangeListener : public ListenerBase {
    virtual ~RangeListener() {}

    inline void analogInput(float value) {
        onChange(value);
    }

    inline void onBinPress() {
        analogInput(1.0f);
    }

    inline void onBinRelease() {
        analogInput(0.0f);
    }

    /**
    The function to override and give custom behavior to.  This is called any time the analog input is changed,
    which should be constantly.  Analog inputs are usually polled at some rate so be ready for that.
    */
    virtual void onChange(float value) {}
};

}

#endif