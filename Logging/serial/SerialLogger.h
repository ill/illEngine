#ifndef SERIAL_LOGGER_H_
#define SERIAL_LOGGER_H_

#include <set>
#include "Logging/Logger.h"

namespace illLogging {

class SerialLogger : public Logger {
public:
    SerialLogger()
        : Logger()
    {}

    virtual ~SerialLogger() {}

    void printMessage(LogDestination::MessageLevel messageLevel, const char * message);

    void addLogDestination(LogDestination * logDestination) {
        m_logDestinations.insert(logDestination);
    }

    void removeLogDestination(LogDestination * logDestination) {
        std::set<LogDestination *>::iterator iter = m_logDestinations.find(logDestination);

        if(iter == m_logDestinations.end()) {
            
        }
        else {
            m_logDestinations.erase(iter);
        }
    }

    bool logDestinationExists(LogDestination * logDestination) const {
        return m_logDestinations.find(logDestination) == m_logDestinations.end();
    }

    void clearLogDestinations() {
        m_logDestinations.clear();
    }

private:
    std::set<LogDestination *> m_logDestinations;
};

}

#endif
