#include <sstream>
#include "DeveloperConsole.h"
#include "VariableManager.h"
#include "CommandManager.h"
#include "Util/util.h"

namespace illConsole {

DeveloperConsole::DeveloperConsole() 
    : LogDestination(), 
    m_outputFileName(DEFAULT_OUTPUT_FILE),
    m_maxLines(DEFUALT_MAX_LINES)
{
    openOutputFile();
}

DeveloperConsole::~DeveloperConsole() {
    closeOutputFile();
}

void DeveloperConsole::printMessage(MessageLevel messageLevel, const char * message) {
    fileOutput(message);

    //prepend with proper color codes
    switch(messageLevel) {
    case MT_ERROR:
    case MT_FATAL:
        m_lines.emplace_back("^1" + std::string(message));
        break;

    case MT_DEBUG:
        m_lines.emplace_back("^5" + std::string(message));
        break;

    default:
        m_lines.emplace_back(message);
        break;
    }

    if(m_lines.size() > m_maxLines) {
        m_lines.pop_front();
    }
}

void DeveloperConsole::consoleDump(const char * fileName) {
    std::ofstream outputFile(fileName);

    for(auto iter = m_lines.begin(); iter != m_lines.end(); iter++) {
        outputFile << *iter;
    }
}

void DeveloperConsole::consoleInput(const char * fileName) {
    std::ifstream inputFile(fileName);
    char line[256];

    while(!inputFile.eof()) {
        inputFile.getline(line, 256);

        if(inputFile.eof()) {
            break;
        }

        //ignore lines starting with # to count as comments
        if(line[0] == '#') {
            continue;
        }

        parseInput(line);
    }
}

bool DeveloperConsole::checkParamEnd(std::istringstream& argStream) {
    if(argStream.eof()) {
        return true;
    }

    std::string dest;
    argStream >> dest;

    if(argStream.eof()) {
        return true;
    }
    else {
        printMessage(illLogging::LogDestination::MessageLevel::MT_ERROR, formatString("Unexpected parameter %s. No more parameters were expected.", dest.c_str()).c_str());
        return false;
    }
}

bool DeveloperConsole::getParamInt(std::istringstream& argStream, int& dest) {
    if(argStream.eof()) {
        printMessage(illLogging::LogDestination::MessageLevel::MT_ERROR, "Expecting an integer parameter. No more parameters were found.");
        return false;
    }

    std::streampos pos = argStream.tellg();

    if(!(argStream >> dest)) {
        printMessage(illLogging::LogDestination::MessageLevel::MT_ERROR, formatString("Expecting an integer parameter at position %d.", pos).c_str());
        return false;
    }

    return true;
}

bool DeveloperConsole::getParamBool(std::istringstream& argStream, bool& dest) {
    if(argStream.eof()) {
        printMessage(illLogging::LogDestination::MessageLevel::MT_ERROR, "Expecting a boolean parameter. No more parameters were found.");
        return false;
    }

    std::streampos pos = argStream.tellg();

    if(!(argStream >> dest)) {
        printMessage(illLogging::LogDestination::MessageLevel::MT_ERROR, formatString("Expecting a boolean parameter at position %d.", pos).c_str());
        return false;
    }

    return true;
}

bool DeveloperConsole::getParamFloat(std::istringstream& argStream, float& dest) {
    if(argStream.eof()) {
        printMessage(illLogging::LogDestination::MessageLevel::MT_ERROR, "Expecting a float parameter. No more parameters were found.");
        return false;
    }

    std::streampos pos = argStream.tellg();

    if(!(argStream >> dest)) {
        printMessage(illLogging::LogDestination::MessageLevel::MT_ERROR, formatString("Expecting a float parameter at position %d.", pos).c_str());
        return false;
    }

    return true;
}

bool DeveloperConsole::getParamString(std::istringstream& argStream, std::string& dest) {
    if(argStream.eof()) {
        printMessage(illLogging::LogDestination::MessageLevel::MT_ERROR, "Expecting a string parameter. No more parameters were found.");
        return false;
    }

    std::streampos pos = argStream.tellg();

    if(!(argStream >> dest)) {
        printMessage(illLogging::LogDestination::MessageLevel::MT_ERROR, formatString("Expecting a string parameter at position %d.", pos).c_str());
        return false;
    }

    return true;
}

void DeveloperConsole::parseInput(const char * input) {
    std::stringstream inputStream(input);

    enum class ParseState {
        BEGINNING,
        COMMAND_ARGS,
        DONE,
    } parseState = ParseState::BEGINNING;

    ConsoleVariable * var = NULL;
    const ConsoleCommand * cmd = NULL;

    while(!inputStream.eof()) {
        switch(parseState) {
        case ParseState::BEGINNING: {
            std::string token;
            inputStream >> token;

            if(inputStream.eof()) {
                break;
            }

            //expect a variable name first or a command
            var = m_variableManager->getVariable(token.c_str());

            if(var) {
                parseState = ParseState::DONE;
                continue;
            }

            cmd = m_commandManager->getCommand(token.c_str());

            if(cmd) {
                parseState = ParseState::COMMAND_ARGS;
                continue;
            }

            printMessage(illLogging::LogDestination::MessageLevel::MT_ERROR, formatString("Invalid variable or command name %s", token.c_str()).c_str());

            } break;

        case ParseState::COMMAND_ARGS:
            cmd->callCommand(inputStream.str().c_str());
            return;

        case ParseState::DONE: {
            std::string token;
            inputStream >> token;

            if(!inputStream.eof()) {
                printMessage(illLogging::LogDestination::MessageLevel::MT_ERROR, "Unexpected tokens after variable name. Using just a variable name prints the value.");
            }

            printMessage(illLogging::LogDestination::MessageLevel::MT_INFO, formatString("%s: %s", var->getValue()).c_str());

            } break;
        }
    }
}

}