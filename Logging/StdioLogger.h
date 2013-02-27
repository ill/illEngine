#ifndef ILL_STDIO_LOGGER_H_
#define ILL_STDIO_LOGGER_H_

#include "Logging/LogDestination.h"

#include <iostream>

namespace illLogging {

class StdioLogger : public LogDestination {
public:
	StdioLogger();
	StdioLogger(const char * path);

	virtual ~StdioLogger() {
		if(m_outputFile != &std::cout) {
		    delete m_outputFile;
        }
	}

	virtual void printMessage(LogDestination::MessageLevel messageLevel, const char * message);

private:
	///The opened file to output log messages to
	std::ostream * m_outputFile;
};

}

#endif