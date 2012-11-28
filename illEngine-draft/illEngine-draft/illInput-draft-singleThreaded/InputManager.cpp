#include "InputManager.h"
#include "../illUtil-draft-singleThreaded/Logging/logging.h"

namespace Input {

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
    std::map<int, InputContextStack>::iterator iter = m_playerToInputContext.find(player);

    if(iter == m_playerToInputContext.end()) {
        LOG_ERROR("Attempting to get input context stack for non-existing player %d in the input manager.", player);
        return NULL;
    }
    else {
        return &(iter->second);
    }
}

}