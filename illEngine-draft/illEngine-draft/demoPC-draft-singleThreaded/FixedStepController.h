#ifndef __FIXED_STEP_CONTROLLER_H__
#define __FIXED_STEP_CONTROLLER_H__

namespace Demo {
class GameControllerBase;
struct Engine;

class FixedStepController {
public:
    FixedStepController(GameControllerBase * gameController, Engine * engine) 
        : m_state(APPST_INITIALIZED),
        m_gameController(gameController),
        m_engine(engine)
    {}

    ~FixedStepController() {}

    void appLoop();

    inline void exitApp() {
        m_state = APPST_EXITING;
    }

private:
    enum State {
        APPST_INITIALIZED,
        APPST_RUNNING,
        APPST_EXITING
    };

    State m_state;

    GameControllerBase * m_gameController;
    Engine * m_engine;
};

}

#endif
