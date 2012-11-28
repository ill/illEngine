#ifndef __MAIN_CONTROLLER_H__
#define __MAIN_CONTROLLER_H__

#include <cstdlib>
#include "../GameControllerBase.h"

namespace Demo {

struct Engine;

class MainController : public GameControllerBase {
public:
    enum State {
        APPST_PREGAME_SPLASH,
        APPST_SINGLE_PLAYER
    };

    MainController(Engine * engine)
        : GameControllerBase(),
        m_engine(engine),
        m_subGame(NULL)
    {
        startMainMenu();
    }

    ~MainController() {
        delete m_subGame;
    }

    void update(float seconds) {
        m_subGame->update(seconds);
    }

    void updateSound(float seconds) {
        m_subGame->updateSound(seconds);
    }

    void render() {
        m_subGame->render();
    }

    void startMainMenu();
    void startSinglePlayer();

private:
    inline void setSubGame(GameControllerBase * subGame) {
        delete m_subGame;
        m_subGame = subGame;
        update(1.0f);
    }

    Engine * m_engine;
    State m_state;
    GameControllerBase * m_subGame;
};

}

#endif