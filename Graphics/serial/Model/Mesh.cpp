#include "Mesh.h"
#include "Graphics/RendererBackend.h"
#include "illEngine/Util/Illmesh/IllmeshLoader.h"

namespace illGraphics {
void Mesh::unload() {
    if(m_state == RES_LOADING) {
        LOG_FATAL_ERROR("Attempting to unload mesh while it's loading");
    }

    if(m_loader) {
        m_loader->unloadMesh(&m_meshBackendData);
        m_loader = NULL;
    }

    delete m_meshFrontendData;

    m_state = RES_UNLOADED;
}

void Mesh::setFrontentDataInternal(MeshData<> * mesh) {
    if(m_state == RES_LOADING) {
        LOG_FATAL_ERROR("Attempting to set mesh frontend while it's loading");
    }

    m_meshFrontendData = mesh;
}

void Mesh::frontendBackendTransferInternal(RendererBackend * loader, bool freeFrontendData) {
    m_loader = loader;
    m_loader->loadMesh(&m_meshBackendData, *m_meshFrontendData);

    if(freeFrontendData) {
        m_meshFrontendData->free();
    }

    m_state = RES_LOADED;
}

void Mesh::reload(RendererBackend * renderer) {
    IllmeshLoader<> meshLoader(m_loadArgs.m_path.c_str());

    setFrontentDataInternal(new MeshData<>(meshLoader.m_numInd / 3, meshLoader.m_numVert, meshLoader.m_features));
    
    meshLoader.buildMesh(*getMeshFrontentData());
    frontendBackendTransferInternal(renderer, true);
}
}
