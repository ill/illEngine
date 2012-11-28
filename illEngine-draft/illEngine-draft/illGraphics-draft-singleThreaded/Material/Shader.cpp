#include "Shader.h"
#include "../RendererBackend.h"

using namespace std;

namespace Graphics {

void Shader::unload() {
    if(m_state == RES_LOADING) {
        LOG_FATAL_ERROR("Attempting to unload shader while it's loading");
    }

    if(m_state == RES_UNINITIALIZED || m_state == RES_UNLOADED) {
        return;
    }

    m_loader->unloadShader(&m_shaderData);

    m_state = RES_UNLOADED;
}

//TODO: Maybe add some error checking to check for invalid masks to this if I have problems
//ex: creating both a fragment shader and a vertex shader, creating two different shaders at a time, stuff like that...
void Shader::reload(RendererBackend * renderer) {
    unload();

    m_loader = renderer;

    m_state = RES_LOADING;

    m_loader->loadShader(&m_shaderData, m_loadArgs);

    m_state = RES_LOADED;
}

void Shader::loadInternal(RendererBackend * renderer, const std::string& path, unsigned int shaderType, const std::string& defines) {
    unload();

    m_loader = renderer;

    m_state = RES_LOADING;

    m_loader->loadShaderInternal(&m_shaderData, path, shaderType, defines);

    m_state = RES_LOADED;
}

}