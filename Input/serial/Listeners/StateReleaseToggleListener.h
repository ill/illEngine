#ifndef ILL_STATE_RELEASE_TOGGLE_LISTENER_H_
#define ILL_STATE_RELEASE_TOGGLE_LISTENER_H_

#include <cstdlib>
#include "Input/serial/Listeners/StateListener.h"

namespace illInput {

/**
This is good for when a value simply needs to be toggled with a button press when the button is released.
*/
struct StateReleaseToggleListener : public StateListener {
    StateReleaseToggleListener()
        : StateListener(),
        m_value(NULL)
    {}

    StateReleaseToggleListener(bool * value)
        : StateListener(),
        m_value(value)
    {}

    virtual ~StateReleaseToggleListener() {}

    void onRelease() {
        *m_value = !*m_value;
    }

    bool * m_value;
};

}

#endif