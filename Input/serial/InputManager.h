#ifndef ILL_INPUT_MANAGER_H__
#define ILL_INPUT_MANAGER_H__

#include <map>
#include <set>

#include "InputContextStack.h"
#include "InputBinding.h"

namespace illInput {

/**
Manages input contexts by player and input devices.

Each player has their own InputContextStack.
A device can map to one or more players.
This way a keyboard can control player 1 and 2 but with different keys.
Or you can hook up multiple controllers or mice.
*/
class InputManager {
public:
    enum class ActionType {
        CONSOLE_COMMAND,
        CONTROL
    };

    void addPlayer(int player);
    void removePlayer(int player);

    bool playerExists(int player) const {
        return m_playerToInputContext.find(player) != m_playerToInputContext.end();
    }

    inline void clearPlayers() {
        m_playerToInputContext.clear();
        m_deviceToPlayer.clear();
    }

    void bindDevice(int device, int player);
    void unbindDevice(int device, int player);

    void bindAction(int player, const InputBinding& inputBinding, const char * action, ActionType actionType);
    void unbindAction(int player, const InputBinding& inputBinding, const char * action);    
    const std::map<std::string, ActionType>* getInputActionBindings(int player, const InputBinding& inputBinding) const;

    inline void clearDevices() {
        m_deviceToPlayer.clear();
    }

    const std::set<int> * getPlayersForDevice(int device);

    InputContextStack * getInputContextStack(int player);

private:
    std::map<int, std::set<int>> m_deviceToPlayer;
    std::map<int, InputContextStack> m_playerToInputContext;

    std::map<int, std::map<InputBinding, std::map<std::string, ActionType>>> m_playerInputActionBindings;
};

}

#endif 