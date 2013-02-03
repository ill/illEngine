#include "InputContext.h"
#include "Logging/logging.h"

namespace illInput {

void InputContext::bindInput(const InputBinding& binding, ListenerBase * input) {
    m_inputMapping[binding] = input;
}

void InputContext::bindInput(const InputBinding& binding, ValueListener * input) {
    m_valueInputMapping[binding] = input;
}

void InputContext::unbindInput(const InputBinding& binding) {
    {
        BindMap::iterator iter = m_inputMapping.find(binding);

        if(iter != m_inputMapping.end()) {
            m_inputMapping.erase(iter);
            return;
        }
    }

    {
        ValueBindMap::iterator iter = m_valueInputMapping.find(binding);

        if(iter != m_valueInputMapping.end()) {
            m_valueInputMapping.erase(iter);
            return;
        }
    }

    LOG_ERROR("Attempting to unbind input for a nonexisting input binding.");
}

ListenerBase * InputContext::lookupBinding(const InputBinding& binding) {
    BindMap::iterator iter = m_inputMapping.find(binding);

    if(iter != m_inputMapping.end()) {
        return iter->second;
    }
    else { 
        return NULL;
    }
}

ValueListener * InputContext::lookupValueBinding(const InputBinding& binding) {
    ValueBindMap::iterator iter = m_valueInputMapping.find(binding);

    if(iter != m_valueInputMapping.end()) {
        return iter->second;
    }
    else {
        return NULL;
    }
}

}
