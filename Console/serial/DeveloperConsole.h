#ifndef ILL_DEVELOPER_CONSOLE_H__
#define ILL_DEVELOPER_CONSOLE_H__

#include <list>
#include <fstream>
#include "Util/util.h"
#include "Logging/LogDestination.h"

namespace illConsole {

static const char * DEFAULT_OUTPUT_FILE = "";
static const size_t DEFUALT_MAX_LINES = 256;

class VariableManager;
class CommandManager;

/**
The developer console allows developers and advanced users to have a
command line interface with the engine.  Average users usually do things
through menus only.
*/
class DeveloperConsole : public illLogging::LogDestination {
public:

    DeveloperConsole();
    ~DeveloperConsole();

    /**
    Process all queued up actions and performs them.
    Run this so messages get printed and commands get executed.
    */
    void update();

    void printMessage(illLogging::LogDestination::MessageLevel messageLevel, const char * message);
    void parseInput(const char * input);
    
    inline void setOutputFile(const char * fileName) {
        if(m_outputFileName == fileName) {
            return;
        }

        closeOutputFile();
        m_outputFileName = fileName;
        openOutputFile();
    }

    inline const char * getOutputFile() const {
        return m_outputFileName.c_str();
    }

    inline void setMaxLines(size_t maxLines) {
        m_maxLines = maxLines;

        while(m_lines.size() > m_maxLines) {
            m_lines.pop_front();
        }
    }

    inline size_t getMaxLines() const {
        return m_maxLines;
    }

    void consoleDump(const char * fileName);
    void consoleInput(const char * fileName);

    inline const std::list<std::string>& getLines() const {
        return m_lines;
    }

    inline void clearLines() {
        m_lines.clear();
    }

    bool checkParamEnd(std::istringstream& argStream);
    bool getParamInt(std::istringstream& argStream, int& dest);
    bool getParamBool(std::istringstream& argStream, bool& dest);
    bool getParamFloat(std::istringstream& argStream, float& dest);
    bool getParamString(std::istringstream& argStream, std::string& dest);

    VariableManager * m_variableManager;
    CommandManager * m_commandManager;

private:
    /**
    Opens a file for writing console output specified by m_outputFileName
    */
    inline void openOutputFile() {
        if(m_outputFileName != "") {
            m_outputFile.open(m_outputFileName.c_str(), std::ios_base::out);
        }
    }

    /**
    Closes the file for writing console output
    */
    inline void closeOutputFile() {
        if(m_outputFile.is_open()) {
            m_outputFile.close();
        }
    }

    /**
    Outputs a message to the open text file if one is open.
    */
    inline void fileOutput(const char * message) {
        if(m_outputFile.is_open()) {
            m_outputFile << message << std::endl;
            m_outputFile.flush();
        }
    }

    size_t m_maxLines;
    std::list<std::string> m_lines;

    ///The filename to output log messages to
    std::string m_outputFileName;

    ///The opened file to output log messages to
    std::ofstream m_outputFile;
};
}

#endif
