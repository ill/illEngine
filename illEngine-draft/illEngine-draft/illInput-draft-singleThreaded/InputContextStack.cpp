#include <cstdlib>

#include "InputContextStack.h"
#include "InputContext.h"
#include "InputBinding.h"

namespace Input {

InputListenerBase * InputContextStack::lookupBinding(const InputBinding& binding) {
    for(std::vector<InputContext *>::reverse_iterator iter = m_stack.rbegin(); iter != m_stack.rend(); iter++) {
        InputListenerBase * retVal = (*iter)->lookupBinding(binding);

        if(retVal != NULL) {
            return retVal;
        }
    }

    return NULL;
}

}
