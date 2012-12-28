#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <set>

#include "serial-illUtil/util.h"
#include "LogDestination.h"

namespace illLogging {

class Logger {
public:
    /**
    Tells the console to log a message.
    @param messageLevel One of the message levels.
    If receiving MT_FATAL, the game will shut down after printing.
    @param message The Message to print. Takes a printf style formatted message with variable arguments.
    */
    void printMessage(LogDestination::MessageLevel messageLevel, const char * message);

    /**
    Tells the console to log a message.
    @param messageLevel One of the message levels.
    If receiving MT_FATAL, the game will shut down after printing.
    @param fileName The source file that originated the message.  Use the macro __FILE__ as the argument value.
    @param lineNumber The line in the source file that originated the message.  Use the macro __LINE__ as the argument value.
    @param message The Message to print. Takes a printf style formatted message with variable arguments.
    Note, after all the formatting takes place, the final message can only be upto 512 characters long.
    */
    inline void printMessage(const char * fileName, const unsigned int lineNumber, LogDestination::MessageLevel messageLevel, const char * message) {
        printMessage(messageLevel, formatString("%s, %u: %s", fileName, lineNumber, message).c_str());
    }

    void addLogDestination(LogDestination * logDestination) {
        m_logDestinations.insert(logDestination);
    }

    void removeLogDestination(LogDestination * logDestination) {
        std::set<LogDestination *>::iterator iter = m_logDestinations.find(logDestination);

        if(iter == m_logDestinations.end()) {
            
        }
        else {
            m_logDestinations.erase(iter);
        }
    }

    bool logDestinationExists(LogDestination * logDestination) const {
        return m_logDestinations.find(logDestination) == m_logDestinations.end();
    }

    void clearLogDestinations() {
        m_logDestinations.clear();
    }

private:
    std::set<LogDestination *> m_logDestinations;
};

//a public global variable, problem?
extern Logger * logger;

}

/**
Put the LOGGER_BEGIN_CATCH macro at the start of a block of code that should have any exceptions be caught by the console and reported as fatal errors
*/
#define LOGGER_BEGIN_CATCH try {

/**
Put the CONS_END_CATCH macro at the end of a block of code that should have any exceptions be caught by the console and reported as fatal errors
@param console a reference to the developer console that will do the error reporting
*/
#define LOGGER_END_CATCH(logger) } catch(std::exception& e) {\
   LOG_FATAL_ERROR(e.what());\
} catch(...) {\
   LOG_FATAL_ERROR("Unknown exception caught");\
}

#endif