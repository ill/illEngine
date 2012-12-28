#ifndef __LOG_DESTINATION_H__
#define __LOG_DESTINATION_H__

namespace illLogging {

class LogDestination {
public:
    /**
    Different kinds of messages the console can receive to print
    */
    enum MessageLevel {
        MT_INFO,          //<Default normal messages printed by the console
        MT_ERROR,         //<A nonfatal recoverable error message.
        MT_FATAL,         //<A fatal unrecoverable error that leads to the application closing.
        MT_DEBUG          //<A debug message that should only show up in debug builds.  Use wrapper macros to log debug messages rather than doing it manually through printMessage since those take care of some stuff.
    };

    virtual void printMessage(MessageLevel messageLevel, const char * message) = 0;
};

}

#endif