#ifndef ILL_INPUT_CONTEXT_H__
#define ILL_INPUT_CONTEXT_H__

#include <string>
#include <map>
#include "InputBinding.h"

namespace illInput {

struct ListenerBase;
struct ValueListener;

/**
Use the InputContextStack to manage these, but you don't have to.
@see InputContextStack

Bind InputBinding objects, which represent some input like a key on a keyboard, to an some action.
*/
struct InputContext {
private:
    typedef std::map<std::string, ListenerBase *> BindMap;
    typedef std::map<std::string, ValueListener *> ValueBindMap;

public:
    void resetListeners();

    void bindInput(const char * action, ListenerBase * input);
    void bindInput(const char * action, ValueListener * input);

    void unbindInput(const char * action);

    /**
    Looks up an input listener bound to an input.
    Returns empty pointer no binding is found.
    */
    ListenerBase * lookupBinding(const char * action);

    /**
    Looks up a value listener bound to an input.
    Returns empty pointer no binding is found.
    */
    ValueListener * lookupValueBinding(const char * action);

private:
    BindMap m_inputMapping;
    ValueBindMap m_valueInputMapping;
};

}

#endif