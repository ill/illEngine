/**
This file needs to be included in the same file that implements the main method.
*/

#include <SDL.h>

#include "serial-illLogging/Logger.h"
#include "illPhysFs/PhysFsFileSystem.h"

#include "Engine.h"

#include "serial-illPC/SdlWindow.h"
#include "serial-illInput/InputManager.h"

#include "serial-illConsole/consoleConsole.h"
#include "serial-illConsole/DeveloperConsole.h"
#include "serial-illConsole/VariableManager.h"

#include "serial-illGraphics/Material/Shader.h"
#include "serial-illGraphics/Material/ShaderProgram.h"
#include "serial-illGraphics/Material/Texture.h"
#include "serial-illGraphics/Material/Material.h"

#include "serial-illGlCommon/GlRenderer.h"

#include "FixedStepController.h"
#include "Game/MainController.h"

//tests
#include "Tests/tests.h"

//statically create all the objects, this way there will be no virtual table overhead from the interfaces
illLogging::Logger thisLogger;
illLogging::Logger * illLogging::logger = &thisLogger;

illPhysFs::PhysFsFileSystem thisFileSystem;
illFileSystem::FileSystem * illFileSystem::fileSystem = &thisFileSystem;

Console::VariableManager consoleVariableManager;
Console::DeveloperConsole developerConsole;

Input::InputManager inputManager;

SdlPc::SdlWindow window;

GlCommon::GlRenderer rendererBackend;

Demo::Engine engine;

int main(int argc, char * argv[]) {
    Console::initConsoleConsole(&consoleVariableManager);    
    illLogging::logger->addLogDestination(&developerConsole);

    LOGGER_BEGIN_CATCH

    //tests
    testVectorManager();
    testEndian();

    thisFileSystem.init(argv[0]);

    //TODO: make the game moddable and able to load the archives from the mod folder
    //TODO: set up creating of the game archive as part of the build
    illFileSystem::fileSystem->addPath("..\\..\\..\\assets");

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
    
    //run game loop
    window.setRenderer(engine.m_rendererBackend);
    window.setInputManager(engine.m_inputManager);
    engine.m_window->initialize();

    Demo::FixedStepController loopController(new Demo::MainController(&engine), &engine);
    loopController.appLoop();

    //uninitialize things
    engine.m_window->uninitialize();
    
    LOGGER_END_CATCH(illLogging::logger)
        
    return 0;
}