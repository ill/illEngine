#include "DeveloperConsole.h"

using namespace Console;

DeveloperConsole::DeveloperConsole() 
    : LogDestination(), 
    m_outputFileName(DEFAULT_OUTPUT_FILE)
{
    if(m_outputFileName != "") {
        openOutputFile();
    }
}

DeveloperConsole::~DeveloperConsole() {
    closeOutputFile();
}

void DeveloperConsole::printMessage(MessageLevel messageLevel, const std::string& message) {
    fileOutput(message);

    //prepend with proper color codes
    /*
    std::string finalMessage = "";

    switch(currentMessage.m_messageLevel) {
    case MT_ERROR:
    break;

    case MT_FATAL:
    break;

    case MT_DEBUG:
    break;
    }*/

    //TODO: add code for displaying console messages
}
