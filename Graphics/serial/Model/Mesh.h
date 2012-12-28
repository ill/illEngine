#ifndef __MESH_H__
#define __MESH_H__

#include "serial-illUtil/Geometry/MeshData.h"

namespace Graphics {

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

    RendererBackend * m_backend;
    MeshData<> * m_meshFrontendData;
    void * m_meshBackendData;
};
}

#endif