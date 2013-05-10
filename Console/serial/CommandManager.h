#ifndef ILL_COMMAND_MANAGER_H_
#define ILL_COMMAND_MANAGER_H_

#include <map>
#include <functional>
#include "Logging/logging.h"

namespace illConsole {

class ConsoleCommand {
public:
    typedef std::function<void (const ConsoleCommand *, const char *)> Callback;

    inline ConsoleCommand(const char * description = "",
        Callback commandFunc = [] (const ConsoleCommand *, const char *) {})
        : m_commandFunc(commandFunc),
        m_description(description)
    {}

    inline const char * getDescription() const {
        return m_description.c_str();
    }

    /**
    Calls the command with some params.
    Params can be NULL to signify no params.
    It's up to the lambda passed in as the function callback to parse the params in some way
    such as having them be separated by spaces or commas or whatever.  I myself will typically
    have it be space separated values.

    This way a command like bind can be written in the console like "bind LEFT_ARROW StrafeLeft"
    and the command func parses the string "LEFT_ARROW StrafeLeft"
    */
    inline void callCommand(const char * params) const {
        m_commandFunc(this, params);
    }

private:
    Callback m_commandFunc;
    std::string m_description;
};

/**
The command manager allows accessing the commands by name.
The functions are just a lambda that takes a char *.
The char * is a single parameter that follows the command and
it's up to the implementing function to parse the command or make it optional
or not required in the first place.

NULL will be passed into the function by the input parser if the person using the console didn't type in parameters afterwards.

The entire string following the command is passed in including any spaces, to allow the function that parses the command to take 
multiple space separated arguments.
*/
class CommandManager {
public:
    CommandManager() {}
    ~CommandManager() {}

    /**
    Adds a command into the system.

    It's possible to add the same command with different names so commands can have aliases or something.
    Duplicate command names aren't allowed.  
    A nonfatal error will be logged if a command with a name already exists and the command won't get added.
    Check for those errors when running the engine to make sure it doesn't happen.

    It's the responsibility of who ever created the command to free it later if it was dynamically allocated.
    The command should only be freed after calling removeCommand or when the program is exiting.
    This is done this way so you can statically create console commands most of the time.

    @param name The command name
    @param variable The command to add
    */
    inline void addCommand(const char * name, const ConsoleCommand* command) {
        //it's helpful to check if the variable name exists so developers don't accidentally make duplicate values
        if(commandExists(name)) {
            LOG_ERROR("Attempting to add console command with an existing name %s", name);

            return;
        }

        m_commands[name] = command;
    }

    /**
    Removes a command from the system.
    This probably shouldn't be used very often.
    This might be useful if some module that was using the command is completely unloaded and the command no longer needs to exist.
    @param name The command name
    */
    inline void removeCommand(const char * name) {
        auto removePos = m_commands.find(name);

        if(removePos != m_commands.end()) {
            m_commands.erase(removePos);
        }
    }

    /**
    Checks if a console command exists.
    @param name The command name
    */
    inline bool commandExists(const char * name) const {
        return m_commands.find(name) != m_commands.end();
    }
    
    /**
    Returns a command so you can call it afterwards.
    @param name The command name
    @return returns NULL if the variable doesn't exist
    */
    inline const ConsoleCommand * getCommand(const char * name) {
        if(!commandExists(name)) {
            return NULL;
        }

        return m_commands[name];
    }

private:
    std::map<std::string, const ConsoleCommand*> m_commands;
};

}

#endif