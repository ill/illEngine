#include "InputManager.h"
#include "Logging/logging.h"

namespace illInput {

void InputManager::addPlayer(int player) {
    if(playerExists(player)) {
        LOG_ERROR("Attempting to add existing player %d to the input manager", player);
    }
    else {
        m_playerToInputContext[player];
    }
}

void InputManager::removePlayer(int player) {
    std::map<int, InputContextStack>::iterator iter = m_playerToInputContext.find(player);

    if(iter == m_playerToInputContext.end()) {
        LOG_ERROR("Attempting to remove non-existing player %d from the input manager", player);
    }
    else {
        m_playerToInputContext.erase(iter);

        //remove all devices for the player
        for(std::map<int, std::set<int> >::iterator deviceIter = m_deviceToPlayer.begin(); deviceIter != m_deviceToPlayer.end(); deviceIter++) {
            deviceIter->second.erase(player);
        }
    }
}

void InputManager::bindDevice(int device, int player) {
    if(!playerExists(player)) {
        LOG_ERROR("Attempting to bind device %d to non-existing player %d in the input manager", device, player);
    }
    else {
        m_deviceToPlayer[device].insert(player);
    }
}

void InputManager::unbindDevice(int device, int player) {
    if(!playerExists(player)) {
        LOG_ERROR("Attempting to unbind device %d from non-existing player %d in the input manager", device, player);
    }
    else {
        if(m_deviceToPlayer[device].erase(player) == 0) {
            LOG_ERROR("Attempting to unbind device %d from player %d in the input manager but the device isn't bound to the player.", device, player);
        }
    }
}

void InputManager::bindAction(int player, const InputBinding& inputBinding, const char * action, ActionType actionType) {
    if(!playerExists(player)) {
        LOG_ERROR("Attempting to bind an action for a non-existing player in the input manager", player);
    }
    else {
        m_playerInputActionBindings[player][inputBinding][action] = actionType;
    }
}

void InputManager::unbindAction(int player, const InputBinding& inputBinding, const char * action) {
    if(!playerExists(player)) {
        LOG_ERROR("Attempting to unbind an action for a non-existing player in the input manager", player);
    }
    else {
        m_playerInputActionBindings[player][inputBinding].erase(action);
    }
}

const std::map<std::string, InputManager::ActionType>* InputManager::getInputActionBindings(int player, const InputBinding& inputBinding) const {
    if(!playerExists(player)) {
        LOG_ERROR("Attempting to get input bindings for non-existing player %d in the input manager.", player);
        return NULL;
    }
    else {
        auto playerIter = m_playerInputActionBindings.find(player);

        if(playerIter == m_playerInputActionBindings.end()) {
            return NULL;
        }

        auto bindingIter = playerIter->second.find(inputBinding);

        if(bindingIter == playerIter->second.end()) {
            return NULL;
        }

        return &(bindingIter->second);
    }
}

const std::set<int> * InputManager::getPlayersForDevice(int device) {
    std::map<int, std::set<int> >::const_iterator deviceIter = m_deviceToPlayer.find(device);

    if(deviceIter == m_deviceToPlayer.end()) {
        return NULL;
    }
    else {
        return &(deviceIter->second);
    }
}

InputContextStack * InputManager::getInputContextStack(int player) {
    auto iter = m_playerToInputContext.find(player);

    if(iter == m_playerToInputContext.end()) {
        LOG_ERROR("Attempting to get input context stack for non-existing player %d in the input manager.", player);
        return NULL;
    }
    else {
        return &(iter->second);
    }
}

}
