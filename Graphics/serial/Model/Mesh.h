#ifndef ILL_MESH_H__
#define ILL_MESH_H__

#include "Util/Geometry/MeshData.h"

namespace illGraphics {

class RendererBackend;

/**
Contains some mesh and its associated vertex buffer objects and index buffer objects.
*/
struct Mesh {
    Mesh(MeshData<> * mesh = NULL)
        : m_meshFrontendData(mesh),
        m_backend(NULL),
        m_meshBackendData(NULL)
    {}

    ~Mesh() {
        cleanBackend();
        delete m_meshFrontendData;
    }

    void frontendBackendTransfer(RendererBackend * backend, bool freeFrontendData = true);
    void cleanBackend();

    inline void * getMeshBackendData() const {
        return m_meshBackendData;
    }

    MeshData<> * m_meshFrontendData;
    RendererBackend * m_backend;
    void * m_meshBackendData;
};
}

#endif