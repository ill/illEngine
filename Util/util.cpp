#include "util.h"
#include <cstdarg>

/**
Maximum length of formatted string after it's all formatted
*/
const int MAX_FORMAT_STRING_LEN = 512;

std::string formatString(const char * formatString, ...) {
    char messageStr[MAX_FORMAT_STRING_LEN];
    va_list args;
    va_start(args, formatString);
    vsnprintf_s(messageStr, MAX_FORMAT_STRING_LEN, _TRUNCATE, formatString, args);
    va_end(args);

    return std::string(messageStr);
}