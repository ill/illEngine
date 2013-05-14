#include "InputContext.h"
#include "Logging/logging.h"

namespace illInput {

void InputContext::bindInput(const char * action, ListenerBase * input) {
    m_inputMapping[action] = input;
}

void InputContext::bindInput(const char * action, ValueListener * input) {
    m_valueInputMapping[action] = input;
}

void InputContext::unbindInput(const char * action) {
    {
        BindMap::iterator iter = m_inputMapping.find(action);

        if(iter != m_inputMapping.end()) {
            m_inputMapping.erase(iter);
            return;
        }
    }

    {
        ValueBindMap::iterator iter = m_valueInputMapping.find(action);

        if(iter != m_valueInputMapping.end()) {
            m_valueInputMapping.erase(iter);
            return;
        }
    }

    LOG_ERROR("Attempting to unbind input for a nonexisting input binding.");
}

ListenerBase * InputContext::lookupBinding(const char * action) {
    BindMap::iterator iter = m_inputMapping.find(action);

    if(iter != m_inputMapping.end()) {
        return iter->second;
    }
    else { 
        return NULL;
    }
}

ValueListener * InputContext::lookupValueBinding(const char * action) {
    ValueBindMap::iterator iter = m_valueInputMapping.find(action);

    if(iter != m_valueInputMapping.end()) {
        return iter->second;
    }
    else {
        return NULL;
    }
}

}
