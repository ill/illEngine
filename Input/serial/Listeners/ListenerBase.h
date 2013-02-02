#ifndef ILL_LISTENER_BASE_H_
#define ILL_LISTENER_BASE_H_

namespace illInput {

/**
Base class for an input action.  This is good for standard inputs like buttons, joysticks, etc...
For more specialized input values like mouse position, phone orientation, etc... use the InputListenerValue object.
*/
struct ListenerBase {
    virtual ~ListenerBase() {}

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
    @param value The analog input value, like joystick axis position, or amount a mouse moved
    */
    virtual void analogInput(float value) = 0;
};

}

#endif