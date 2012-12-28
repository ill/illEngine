#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <stdint.h>

namespace Console {
class DeveloperConsole;
class VariableManager;
}

template <typename Key, typename T, typename Loader> class ResourceManager;
template<typename Id, typename T, typename LoadArgs, typename Loader> class ConfigurableResourceManager;

namespace Graphics {
class Window;
class RendererBackend;

class Shader;
typedef ResourceManager<uint64_t, Shader, RendererBackend> ShaderManager;

class ShaderProgram;
struct ShaderProgramLoader;
typedef ResourceManager<uint64_t, ShaderProgram, ShaderProgramLoader> ShaderProgramManager;

class Texture;
struct TextureLoadArgs;
typedef ConfigurableResourceManager<uint32_t, Texture, TextureLoadArgs, RendererBackend> TextureManager;

class Material;
class MaterialManager;

//class Mesh;
//class MeshManager;
}

namespace Input {
class InputManager;
}

namespace Demo {

/**
TODO: this is generally a good struct for any simple game with a single window so put this in util sometime or something
*/
struct Engine {
public:
    Console::DeveloperConsole * m_developerConsole;
    Console::VariableManager * m_consoleVariableManager;

    Graphics::Window * m_window;
    Graphics::RendererBackend * m_rendererBackend;

    Graphics::ShaderManager * m_shaderManager;
    Graphics::ShaderProgramManager * m_shaderProgramManager;
    Graphics::TextureManager * m_textureManager;
    //Graphics::MaterialManager * m_materialProgramManager;
    //Graphics::MeshManager * m_meshManager;

    Input::InputManager * m_inputManager;
};

}

#endif