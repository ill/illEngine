#include <cstdlib>

#include "InputContextStack.h"
#include "InputContext.h"
#include "InputBinding.h"

namespace illInput {

InputContext * InputContextStack::popInputContext() {
    InputContext * res = m_stack.back();

    //reset states of all input listeners
    res->resetListeners(); 
    m_stack.pop_back();

    return res;
}

bool InputContextStack::findInputContextStackPos(InputContext * input, size_t& dest) {
    for(size_t pos = 0; pos < m_stack.size(); pos++) {
        if(m_stack[pos] == input) {
            dest = pos;
            return true;
        }
    }
    
    return false;
}

void InputContextStack::replaceInputContext(InputContext * input, size_t stackPos) {
    m_stack[stackPos] = input;
}

ListenerBase * InputContextStack::lookupBinding(const char *action) {
    for(std::vector<InputContext *>::reverse_iterator iter = m_stack.rbegin(); iter != m_stack.rend(); iter++) {
        ListenerBase * retVal = (*iter)->lookupBinding(action);

        if(retVal != NULL) {
            return retVal;
        }
    }

    return NULL;
}

ValueListener * InputContextStack::lookupValueBinding(const char * action) {
    for(std::vector<InputContext *>::reverse_iterator iter = m_stack.rbegin(); iter != m_stack.rend(); iter++) {
        ValueListener * retVal = (*iter)->lookupValueBinding(action);

        if(retVal != NULL) {
            return retVal;
        }
    }

    return NULL;
}

}
