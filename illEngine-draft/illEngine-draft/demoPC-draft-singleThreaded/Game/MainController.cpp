#include "MainController.h"
#include "MainMenu/MainMenuController.h"

namespace Demo {

void MainController::startMainMenu() {
   m_state = APPST_PREGAME_SPLASH;
   setSubGame(new MainMenuController(m_engine));
}

void MainController::startSinglePlayer() {
   m_state = APPST_SINGLE_PLAYER;
   //setSubGame(new SinglePlayer::SinglePlayerGameController(this));
}

}