/**
This excellently named header file holds the console variable and command
definitions of relating to the developer console itself.
*/

#ifndef __CONSOLECONSOLE_H__
#define __CONSOLECONSOLE_H__

#include "ConsoleVariable.h"
#include "DeveloperConsole.h"

namespace Console {
extern ConsoleVariable c_outputFile;

///adds the console variables and commands to the managers, the managers first needs to exist for this to work
void initConsoleConsole(Console::VariableManager * consoleVariableManager);

}

#endif