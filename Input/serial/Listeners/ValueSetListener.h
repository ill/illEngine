#ifndef ILL_VALUE_SET_LISTENER_H_
#define ILL_VALUE_SET_LISTENER_H_

#include <cstdlib>
#include "Input/serial/Listeners/ValueListener.h"

namespace illInput {

/**
This is good for when a value simply needs to come in and be assigned.
*/
struct ValueSetListener : public ValueListener {
    ValueSetListener()
        : ValueListener(),
        m_value(NULL)
    {}

    ValueSetListener(void ** value)
        : ValueListener(),
        m_value(value)
    {}

    virtual ~ValueSetListener() {}

    void onChange(CopiedData value) {
        *m_value = realloc(*m_value, value.m_size);
        memcpy(*m_value, value.m_data, value.m_size);
    }

    void ** m_value;
};

}

#endif