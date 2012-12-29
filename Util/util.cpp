#include "util.h"
#include <cstdarg>
#include <cstdio>

/**
Maximum length of formatted string after it's all formatted
*/
const int MAX_FORMAT_STRING_LEN = 512;

std::string formatString(const char * formatString, ...) {
    char messageStr[MAX_FORMAT_STRING_LEN];
    va_list args;
    va_start(args, formatString);

#ifdef _MSC_VER
    vsnprintf_s(messageStr, MAX_FORMAT_STRING_LEN, _TRUNCATE, formatString, args);
#else
    vsnprintf(messageStr, MAX_FORMAT_STRING_LEN, formatString, args);
#endif

    va_end(args);

    return std::string(messageStr);
}
