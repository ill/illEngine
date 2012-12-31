#include "Mesh.h"
#include "Graphics/RendererBackend.h"

namespace illGraphics {
void Mesh::cleanBackend() {
    if(m_backend) {
        m_backend->unloadMesh(&m_meshBackendData);
        m_backend = NULL;
    }
}

void Mesh::frontendBackendTransfer(RendererBackend * backend, bool freeFrontendData) {
    m_backend = backend;
    m_backend->loadMesh(&m_meshBackendData, *m_meshFrontendData);

    if(freeFrontendData) {
        m_meshFrontendData->free();
    }
}
}
