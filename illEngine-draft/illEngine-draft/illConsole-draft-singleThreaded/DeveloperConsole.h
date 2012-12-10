#ifndef __DEVELOPERCONSOLE_H__
#define __DEVELOPERCONSOLE_H__

#include <fstream>
#include "../illUtil-draft-singleThreaded/util.h"
#include "../illUtil-draft-singleThreaded/Logging/LogDestination.h"

namespace Console {

static const char * DEFAULT_OUTPUT_FILE = "consoleOut.txt";   //TODO: This should normally be a blank string to disable output

/**
The developer console allows developers and advanced users to have a
command line interface with the engine.  Average users usually do things
through menus only.
*/
class DeveloperConsole : public LogDestination {
public:
   
   DeveloperConsole();
   virtual ~DeveloperConsole();

   /**
   Process all queued up actions and performs them.
   Run this so messages get printed and commands get executed.
   */
   void update();

   void printMessage(MessageLevel messageLevel, const std::string& message);
      
private:
   /**
   Opens a file for writing console output specified by m_outputFileName
   */
   inline void openOutputFile() {
      m_outputFile.open(m_outputFileName.c_str(), std::ios_base::out);
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
   inline void fileOutput(const std::string& message) {
      if(m_outputFile.is_open()) {
         m_outputFile << message << std::endl;
         m_outputFile.flush();
      }
   }

   ///The filename to output log messages to
   std::string m_outputFileName;

   ///The opened file to output log messages to
   std::ofstream m_outputFile;
};
}

#endif
