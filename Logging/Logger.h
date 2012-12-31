/*
 * Logger.h
 *
 *  Created on: Dec 29, 2012
 *      Author: Ilya
 */

#ifndef ILL_LOGGER_H_
#define ILL_LOGGER_H_

#include "Util/util.h"
#include "Logging/LogDestination.h"

namespace illLogging {
/**
 * The logger is what umm, does all the logging.
 * You should, in most cases, use the macros in logging.h since they are nice wrappers around these methods.
 * The logger itself doesn't do the logging, it contains LogDestination objects.
 * You should add LogDestinations to actually see results of logging.
 * Those can be developer consoles, output, Android or iOS debug message loggers, etc...
 *
 * Your application should typically have one of these and the global variable below should point to it.
 */
class Logger {
public:
    /**
    Tells the console to log a message.
    @param messageLevel One of the message levels.
    If receiving MT_FATAL, the game will shut down after printing.
    @param message The Message to print. Takes a printf style formatted message with variable arguments.
    */
    virtual void printMessage(LogDestination::MessageLevel messageLevel, const char * message) = 0;

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

    virtual void addLogDestination(LogDestination * logDestination) = 0;

    virtual void removeLogDestination(LogDestination * logDestination) = 0;

    virtual bool logDestinationExists(LogDestination * logDestination) const = 0;

    virtual void clearLogDestinations() = 0;
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

#endif /* LOGGER_H_ */
