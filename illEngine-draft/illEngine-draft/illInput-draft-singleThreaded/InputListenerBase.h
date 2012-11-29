#ifndef __INPUT_LISTENER_BASE_H__
#define __INPUT_LISTENER_BASE_H__

namespace Input {

/**
Base class for an input action
*/
struct InputListenerBase {
    virtual ~InputListenerBase() {}

    /**
    Happens when a button of some sort is pressed
    */
    virtual void onBinPress() = 0;

    /**
    Happens when a button of some sort is released
    */
    virtual void onBinRelease() = 0;

    /**
    Receives a continuous analog input like from mouse motion or from a joystick
    @param value The analog input value, like joystick position, or amount a mouse moved
    */
    virtual void analogInput(float value) = 0;
};

}

#endif