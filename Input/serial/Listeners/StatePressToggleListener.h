#ifndef ILL_STATE_PRESS_TOGGLE_LISTENER_H_
#define ILL_STATE_PRESS_TOGGLE_LISTENER_H_

#include <cstdlib>
#include "Input/serial/Listeners/StateListener.h"

namespace illInput {

/**
This is good for when a value simply needs to be toggled with a button press when the button is first pressed.
*/
struct StatePressToggleListener : public StateListener {
    StatePressToggleListener()
        : StateListener(),
        m_value(NULL)
    {}

    StatePressToggleListener(bool * value)
        : StateListener(),
        m_value(value)
    {}

    virtual ~StatePressToggleListener() {}

    void onPress() {
        *m_value = !*m_value;
    }

    bool * m_value;
};

}

#endif