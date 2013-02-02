#ifndef ILL_RANGE_SET_LISTENER_H_
#define ILL_RANGE_SET_LISTENER_H_

#include <cstdlib>
#include "Input/serial/Listeners/RangeListener.h"

namespace illInput {

/**
This is good for when a value simply needs to come in and be assigned.
*/
struct RangeSetListener : public RangeListener {
    RangeSetListener()
        : StateListener(),
        m_value(NULL)
    {}

    RangeSetListener(float * value)
        : RangeListener(),
        m_value(value)
    {}

    virtual ~StateSetListener() {}

    void onChange(float value) {
        *m_value = value;
    }

    float * m_value;
};

}

#endif