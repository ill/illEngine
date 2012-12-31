#ifndef ILL_INPUT_CONTEXT_STACK_H__
#define ILL_INPUT_CONTEXT_STACK_H__

#include <vector>

namespace Input {

struct InputContext;
struct InputBinding;
struct InputListenerBase;

/**
Keeps track of a stack of bound inputs.  Whatever object polls for inputs can hook in through this to let those inputs get handled.

Here is an example usage:
This is useful so you can push onto the stack the pause button for example so it's always there.
Then push the weapon firing controls.
Then push a character's movement controls.

Now if the character enters a vehicle, you can pop the movement controls and push the vehicle controls.

This way the pause button is always bound, the weapon firing controls are always bound,
and you can easily push and pop different movement controls.

The possibilities are endless...

The whole input system and this stack are based on ideas from this article: http://www.gamedev.net/blog/355/entry-2250186-designing-a-robust-input-handling-system-for-games/
*/
class InputContextStack {
public:
    inline void pushInputContext(InputContext * input) {
        m_stack.push_back(input);
    }

    inline void popInputContext() {
        m_stack.pop_back();
    }

    /**
    Looks up an input listener bound to an input based on contexts on the stack.
    Returns NULL if no binding is found.
    */
    InputListenerBase * lookupBinding(const InputBinding& binding);

private:
    std::vector<InputContext *> m_stack;
};

}

#endif