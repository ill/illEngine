#include <cstdlib>

#include "InputContextStack.h"
#include "InputContext.h"
#include "InputBinding.h"

namespace illInput {

ListenerBase * InputContextStack::lookupBinding(const InputBinding& binding) {
    for(std::vector<InputContext *>::reverse_iterator iter = m_stack.rbegin(); iter != m_stack.rend(); iter++) {
        ListenerBase * retVal = (*iter)->lookupBinding(binding);

        if(retVal != NULL) {
            return retVal;
        }
    }

    return NULL;
}

ValueListener * InputContextStack::lookupValueBinding(const InputBinding& binding) {
    for(std::vector<InputContext *>::reverse_iterator iter = m_stack.rbegin(); iter != m_stack.rend(); iter++) {
        ValueListener * retVal = (*iter)->lookupValueBinding(binding);

        if(retVal != NULL) {
            return retVal;
        }
    }

    return NULL;
}

}
