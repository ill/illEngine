/**
A bunch of macros useful for logging debug messages and all that.

For best results, do all logging through here and never call the logging functions directly unless there's a very good reason.

For info on why I wrote do {} while(0) go here http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert/
*/

#ifndef __LOGGING_H__
#define __LOGGING_H__

#include "Logger.h"
#include "LogDestination.h"

///////////////////////////////
#ifndef NDEBUG    //Debug Build

#define LOG_ERROR(message, ...) do {\
   logger->printMessage(__FILE__, __LINE__, LogDestination::MT_ERROR, formatString((message), ##__VA_ARGS__));\
} while(0)

#define LOG_FATAL_ERROR(message, ...) do {\
    logger->printMessage(__FILE__, __LINE__, LogDestination::MT_FATAL, formatString((message), ##__VA_ARGS__));\
} while(0)

//It's the debug build, so no harm in printing file and line number even in generic info messages, might be useful sometimes
#define LOG_INFO(message, ...) do {\
   logger->printMessage(__FILE__, __LINE__, LogDestination::MT_INFO, formatString((message), ##__VA_ARGS__));\
} while(0)

#define LOG_DEBUG(message, ...) do {\
   logger->printMessage(__FILE__, __LINE__, LogDestination::MT_DEBUG, formatString((message), ##__VA_ARGS__));\
} while(0)

/////////////////////////////////
#else             //Release Build

#define LOG_ERROR(message, ...) do {\
   logger->printMessage(LogDestination::MT_ERROR, formatString((message), ##__VA_ARGS__));\
} while(0)

//it's still useful to allow fatal error messages to print file and line number even in release build
#define LOG_FATAL_ERROR(message, ...) do {\
   logger->printMessage(__FILE__, __LINE__, LogDestination::MT_FATAL, formatString((message), ##__VA_ARGS__));\
} while(0)

//probably don't want info messages to print file and line numbers in release build
#define LOG_INFO(message, ...) do {\
   logger->printMessage(LogDestination::MT_INFO, formatString((message), ##__VA_ARGS__));\
} while(0)

//in the release build, debug output code doesn't even compile in because the compiler optimizes out code that does nothing useful
#define LOG_DEBUG(message, ...) do {\
   (void)sizeof(message);\
} while(0)

///////////////////////////////
#endif            //end if else


#endif