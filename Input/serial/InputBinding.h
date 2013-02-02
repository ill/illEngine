#ifndef ILL_INPUT_BINDING_H__
#define ILL_INPUT_BINDING_H__

namespace illInput {

/**
A general input binding identifier that can represent a keyboard button, a controller button, a joystick axis, etc...
*/
struct InputBinding {
    int m_deviceType;               //helps differentiate between different device types, such as keyboard, mouse, conroller, touch screen, etc...
    int m_input;                    //which input on the device was it, a key on a keyboard, button on a controller, axis on a mouse, etc...

    InputBinding() {}

    InputBinding(int deviceType, int input)
        : m_deviceType(deviceType),
        m_input(input)
    {}

    /**
    Implemented for use as a key in maps
    */
    bool operator<(const InputBinding& other) const {
        if(m_deviceType == other.m_deviceType) {
            return m_input < other.m_input;
        }
                
        return m_deviceType < other.m_deviceType;
    }
};

}

#endif