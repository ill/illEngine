/**
This file needs to be included in the same file that implements the main method.
*/

#include <SDL.h>

#include "../illUtil-draft-singleThreaded/Logging/Logger.h"

#include "Engine.h"

#include "../illPC-draft-singleThreaded/SdlWindow.h"
#include "../illInput-draft-singleThreaded/InputManager.h"

#include "../illConsole-draft-singleThreaded/consoleConsole.h"
#include "../illConsole-draft-singleThreaded/DeveloperConsole.h"
#include "../illConsole-draft-singleThreaded/VariableManager.h"

#include "../illGraphics-draft-singleThreaded/Material/Shader.h"
#include "../illGraphics-draft-singleThreaded/Material/ShaderProgram.h"
#include "../illGraphics-draft-singleThreaded/Material/Texture.h"
#include "../illGraphics-draft-singleThreaded/Material/Material.h"

#include "../illGlCommon-draft-singleThreaded/GlRenderer.h"

#include "../illUtil-draft-singleThreaded/physFsUtil.h"

#include "FixedStepController.h"
#include "Game/MainController.h"

//statically create all the objects, this way there will be no virtual table overhead from the interfaces
Logger thisLogger;
Logger * logger = &thisLogger;

Console::VariableManager consoleVariableManager;
Console::DeveloperConsole developerConsole;

Input::InputManager inputManager;

SdlPc::SdlWindow window;

GlCommon::GlRenderer rendererBackend;

Demo::Engine engine;

int main(int argc, char * argv[]) {
    Console::initConsoleConsole(&consoleVariableManager);    
    logger->addLogDestination(&developerConsole);

    LOGGER_BEGIN_CATCH

    engine.m_window = &window;
    engine.m_developerConsole = &developerConsole;
    engine.m_consoleVariableManager = &consoleVariableManager;
    engine.m_inputManager = &inputManager;
    engine.m_rendererBackend = &rendererBackend;

    Graphics::ShaderManager shaderManager(engine.m_rendererBackend);

    Graphics::ShaderProgramLoader shaderProgramLoader(engine.m_rendererBackend, &shaderManager);
    Graphics::ShaderProgramManager shaderProgramManager(&shaderProgramLoader);
    Graphics::TextureManager textureManager(engine.m_rendererBackend);
    //Graphics::MaterialManager materialProgramManager;

    engine.m_shaderManager = &shaderManager;
    engine.m_shaderProgramManager = &shaderProgramManager;
    engine.m_textureManager = &textureManager;

    //init PhysFS and load the archive
    if(!PHYSFS_init(argv[0])) {
        LOG_FATAL_ERROR("Failed to initialize physFS with error %s", PHYSFS_getLastError());
    }

    //TODO: make the game moddable and able to load the archives from the mod folder
    //TODO: set up creating of the game archive as part of the build
    if(!PHYSFS_addToSearchPath("..\\..\\..\\assets", true)) {
        LOG_FATAL_ERROR("Failed to add assets to search path with error %s", PHYSFS_getLastError());
    }

    //run game loop
    window.setRenderer(engine.m_rendererBackend);
    window.setInputManager(engine.m_inputManager);
    engine.m_window->initialize();

    Demo::FixedStepController loopController(new Demo::MainController(&engine), &engine);
    loopController.appLoop();

    engine.m_window->uninitialize();

    //uninitialize things
    if (!PHYSFS_deinit()) {
        LOG_FATAL_ERROR("Failed to uninitialize physFS with error %s", PHYSFS_getLastError());
    }
    
    LOGGER_END_CATCH(logger)
        
    return 0;
}