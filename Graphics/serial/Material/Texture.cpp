#include "Graphics/serial/Material/Texture.h"
#include "Graphics/GraphicsBackend.h"

#include "Logging/logging.h"

namespace illGraphics {

void Texture::unload() {
    if(m_state == RES_LOADING) {
        LOG_FATAL_ERROR("Attempting to unload texture while it's loading");
    }

    if(m_state == RES_UNINITIALIZED || m_state == RES_UNLOADED) {
        return;
    }

    m_loader->unloadTexture(&m_textureData);

    m_state = RES_UNLOADED;
}

void Texture::reload(GraphicsBackend * backend) {
    unload();

    m_loader = backend;

    m_state = RES_LOADING;

    m_loader->loadTexture(&m_textureData, m_loadArgs);

    m_state = RES_LOADED;
}

}
