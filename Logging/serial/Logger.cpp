#include <SDL_assert.h>		//TODO: make this non platform dependant later
#include "Logger.h"

const std::string ERROR_MESSAGE("ERROR: ");
const std::string FATAL_ERROR_MESSAGE("FATAL ERROR: ");
const std::string DEBUG_MESSAGE("Debug: ");

namespace illLogging {

void Logger::printMessage(LogDestination::MessageLevel messageLevel, const char * message) {	
    std::string finalMessage = "";

    //prepend with proper prefixes
    switch(messageLevel) {
    case LogDestination::MT_ERROR:
        finalMessage.append(ERROR_MESSAGE).append(message);
        break;

    case LogDestination::MT_FATAL:
        finalMessage.append(FATAL_ERROR_MESSAGE).append(message);
        break;

    case LogDestination::MT_DEBUG:
        finalMessage.append(DEBUG_MESSAGE).append(message);
        break;

    default:
        finalMessage = message;
    }

    //log to all log destinations
    for(std::set<LogDestination *>::iterator iter = m_logDestinations.begin(); iter != m_logDestinations.end(); iter++) {
        (*iter)->printMessage(messageLevel, finalMessage.c_str());
    }
    
    //if fatal error message, just exit the application
    if(messageLevel == LogDestination::MT_FATAL) {
        //if debugging, cause a breakpoint so I can do a stack trace
        SDL_TriggerBreakpoint();

        exit(-1);
    }
}

}