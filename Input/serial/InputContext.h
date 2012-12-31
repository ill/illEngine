#ifndef ILL_INPUT_CONTEXT_H__
#define ILL_INPUT_CONTEXT_H__

#include <map>
#include "InputBinding.h"

namespace Input {

struct InputListenerBase;

/**
Use the InputContextStack to manage these, but you don't have to.
@see InputContextStack

Bind InputBinding objects, which represent some input like a key on a keyboard, to an some action.
*/
struct InputContext {
private:
    typedef std::map<InputBinding, InputListenerBase *> BindMap;

public:
    void bindInput(const InputBinding& binding, InputListenerBase * input);

    void unbindInput(const InputBinding& binding);

    /**
    Looks up an input listener bound to an input.
    Returns NULL if no binding is found.
    */
    InputListenerBase * lookupBinding(const InputBinding& binding);

private:
    BindMap m_inputMapping;
};

}

#endif