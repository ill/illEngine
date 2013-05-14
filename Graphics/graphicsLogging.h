#ifndef ILL_GRAPHICS_LOGGING_H__
#define ILL_GRAPHICS_LOGGING_H__

/*
Logger stuff for communication
*/

#include "Logging/logging.h"

//whether or not to log video debug messages
#define ENABLE_LOG_DEBUG_GRAPHICS 0

///////////////////////////////
#if ENABLE_LOG_DEBUG_GRAPHICS

#define LOG_DEBUG_GRAPHICS(message, ...) do {\
   LOG_DEBUG((message), __VA_ARGS__);\
} while(0)

///////////////////////////////
#else

#define LOG_DEBUG_GRAPHICS(message, ...) do {\
   sizeof(message);\
} while(0)

///////////////////////////////
#endif


//whether or not to log video resource stream messages
#define ENABLE_LOG_DEBUG_GRAPHICS_STREAM 1

///////////////////////////////
#if ENABLE_LOG_DEBUG_GRAPHICS_STREAM

#define LOG_DEBUG_GRAPHICS_STREAM(message, ...) do {\
   LOG_DEBUG((message), __VA_ARGS__);\
} while(0)

///////////////////////////////
#else

#define LOG_DEBUG_GRAPHICS_STREAM(message, ...) do {\
   sizeof(message);\
} while(0)

///////////////////////////////
#endif

#endif
