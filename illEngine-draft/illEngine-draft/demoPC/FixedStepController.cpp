#include <stdint.h>
#include <SDL.h>

#include "FixedStepController.h"
#include "GameControllerBase.h"

#include "Engine.h"
#include "serial-illGraphics/Window.h"
#include "serial-illGraphics/RendererBackend.h"
#include "serial-illLogging/logging.h"

namespace Demo {

const float STEP_SIZE = 1000.0f / 60.0f;     //makes application run at a base FPS of 60
const int MAX_STEPS = 10;

void FixedStepController::appLoop() {      
    //if the application controller isn't in the expected state error
    if (m_state != APPST_INITIALIZED) {
        LOG_FATAL_ERROR("Application in unexpected state.");
    }

    m_state = APPST_RUNNING;

    float timeAccumulator = 0.0f;
    int32_t lastLoopTime = SDL_GetTicks();

    while (m_state == APPST_RUNNING) {
        //poll input events from the game's window
        m_engine->m_window->pollEvents();

        //compute time since last game loop
        int32_t currentLoopTime = SDL_GetTicks();
        int32_t milliseconds = currentLoopTime - lastLoopTime;
        lastLoopTime = currentLoopTime;

        timeAccumulator += milliseconds;
        
        int steps = 0;

        //run the game loop with a fixed step
        while(timeAccumulator > STEP_SIZE && steps++ < MAX_STEPS) {
            timeAccumulator -= STEP_SIZE;

            m_gameController->update(STEP_SIZE / 1000.0f);
        }
        
        /////////////////////
        //update sound
        m_gameController->updateSound(milliseconds / 1000.0f);

        /////////////////////
        //draw screen
        m_engine->m_window->beginFrame();
        m_engine->m_rendererBackend->beginFrame();

        m_gameController->render();

        m_engine->m_rendererBackend->endFrame();
        m_engine->m_window->endFrame();

        //force delay 1 ms to avoid precision issues
        SDL_Delay(1);
    }
}

}