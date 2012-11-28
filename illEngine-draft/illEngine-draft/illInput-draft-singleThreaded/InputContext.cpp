#include "InputContext.h"
#include "../illUtil-draft-singleThreaded/Logging/logging.h"

namespace Input {

void InputContext::bindInput(const InputBinding& binding, InputListenerBase * input) {
    m_inputMapping[binding] = input;
}

void InputContext::unbindInput(const InputBinding& binding) {
    BindMap::iterator iter = m_inputMapping.find(binding);

    if(iter != m_inputMapping.end()) {
        m_inputMapping.erase(iter);
    }
    else {
        LOG_ERROR("Attempting to unbind input for a nonexisting input binding.");
    }
}

InputListenerBase * InputContext::lookupBinding(const InputBinding& binding) {
    BindMap::iterator iter = m_inputMapping.find(binding);

    if(iter != m_inputMapping.end()) {
        return iter->second;
    }
    else {
        return NULL;
    }
}

}