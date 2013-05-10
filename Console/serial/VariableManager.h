#ifndef ILL_VARIABLEMANAGER_H__
#define ILL_VARIABLEMANAGER_H__

#include <string>
#include <functional>
#include <map>
#include "Logging/logging.h"

namespace illConsole {

/**
The console variable itself.
*/
class ConsoleVariable {
public:
    typedef std::function<bool (ConsoleVariable *, const char *)> Callback;

    inline ConsoleVariable(const char * value, const char * description = "",
        Callback onChangeCallback = [] (ConsoleVariable *, const char *) {return true;})
        : m_value(value),
        m_description(description),
        m_onChangeCallback(onChangeCallback)
    {}

    inline const char * getValue() const {
        return m_value.c_str();
    }
    
    inline void setValue(const char * value) {
        if(m_onChangeCallback(this, value)) {
            m_value = value;
        }
    }

    inline const char * getDescription() const {
        return m_description.c_str();
    }

private:
    ///a callback lambda for when a change occurs
    Callback m_onChangeCallback;
    
    std::string m_value;

    ///variable description that shows up when someone views it in the dev console
    std::string m_description;
};

/**
The variable manager allows accessing the variables by name.
Application settings should be stored here.
*/
class VariableManager {
public:
    VariableManager() {}
    ~VariableManager() {}

    /**
    Adds a variable into the system.

    It's possible to add the same variable with different names so variables can have aliases or something.
    Duplicate variable names aren't allowed.  
    A nonfatal error will be logged if a variable with a name already exists and the variable won't get added.
    Check for those errors when running the engine to make sure it doesn't happen.

    It's the responsibility of who ever created the variable to free it later if it was dynamically allocated.
    The variable should only be freed after calling removeVariable or when the program is exiting.
    This is done this way so you can statically create console variables most of the time.

    @param name The variable name
    @param variable The variable to add
    */
    inline void addVariable(const char * name, ConsoleVariable * variable) {
        //it's helpful to check if the variable name exists so developers don't accidentally make duplicate values
        if(variableExists(name)) {
            LOG_ERROR("Attempting to add console variable with an existing name %s", name);

            return;
        }

        m_variables[name] = variable;
    }

    /**
    Removes a variable from the system.
    This probably shouldn't be used very often.
    This might be useful if some module that was using the variable is completely unloaded and the variable no longer needs to exist.
    @param name The variable name
    */
    inline void removeVariable(const char * name) {
        auto removePos = m_variables.find(name);

        if(removePos != m_variables.end()) {
            m_variables.erase(removePos);
        }
    }

    /**
    Checks if a console variable exists.
    @param name The variable name
    */
    inline bool variableExists(const char * name) const {
        return m_variables.find(name) != m_variables.end();
    }

    /**
    Returns a variable so you can do with it whatever you want, read it, modify it, whatever...
    @param name The variable name
    @return returns NULL if the variable doesn't exist

    //TODO: rewrite this completely.  It shouldn't work this way.  
    (Why?  I didn't tell myself.  Probably due to thread safety.  Should be fine for serialized version)
    */
    inline ConsoleVariable * getVariable(const char * name) {
        if(!variableExists(name)) {
            return NULL;
        }

        return m_variables[name];
    }

private:
    std::map<std::string, ConsoleVariable *> m_variables;
};

}

#endif
