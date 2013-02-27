#include "Logging/StdioLogger.h"
#include <fstream>

namespace illLogging {
StdioLogger::StdioLogger() {
	m_outputFile = &std::cout;
}

StdioLogger::StdioLogger(const char * path) {
	m_outputFile = new std::ofstream(path);
}

void StdioLogger::printMessage(LogDestination::MessageLevel messageLevel, const char * message) {
	(*m_outputFile) << message << std::endl;
    m_outputFile->flush();
}

}