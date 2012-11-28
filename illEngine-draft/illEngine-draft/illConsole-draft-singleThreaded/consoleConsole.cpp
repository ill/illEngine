#include "VariableManager.h"
#include "consoleConsole.h"

Console::ConsoleVariable Console::c_outputFile(new ConsoleVariableString(DEFAULT_OUTPUT_FILE), 
   "The filename to output all console messages to.  Leave blank for no output.");

void Console::initConsoleConsole(Console::VariableManager * consoleVariableManager) {
   consoleVariableManager->addVariable("con_outputFile", &c_outputFile);
}