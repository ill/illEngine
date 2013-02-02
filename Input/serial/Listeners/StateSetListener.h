#ifndef ILL_STATE_SET_LISTENER_H_
#define ILL_STATE_SET_LISTENER_H_

#include <cstdlib>
#include "Input/serial/Listeners/StateListener.h"

namespace illInput {

/**
This is good for when a value simply needs to come in and be assigned.
*/
struct StateSetListener : public StateListener {
    StateSetListener()
        : StateListener(),
        m_value(NULL)
    {}

    StateSetListener(bool * value)
        : StateListener(),
        m_value(value)
    {}

    virtual ~StateSetListener() {}

    void onChange(bool value) {
        *m_value = value;
    }

    bool * m_value;
};

}

#endif