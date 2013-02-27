#ifndef ILL_MESH_H__
#define ILL_MESH_H__

#include <string>
#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"
#include "Util/Geometry/MeshData.h"

namespace illGraphics {

class RendererBackend;


struct MeshLoadArgs {
    std::string m_path; //path of mesh file

    //TODO: more to come?  Maybe?
};

/**
Contains some mesh and its associated vertex buffer objects and index buffer objects.
*/
struct Mesh {
    Mesh(MeshData<> * mesh = NULL)
        : m_meshFrontendData(mesh),
        m_backend(NULL),
class Mesh : public ResourceBase<Mesh, MeshLoadArgs, RendererBackend> {
public:
    Mesh()
        : m_meshFrontendData(NULL),
        m_meshBackendData(NULL)
    {}

    ~Mesh() {
        cleanBackend();
        delete m_meshFrontendData;
        unload();
    }

    void frontendBackendTransfer(RendererBackend * backend, bool freeFrontendData = true);
    void cleanBackend();
    virtual void unload();
    virtual void reload(RendererBackend * renderer);

    void setFrontentDataInternal(MeshData<> * mesh);
    void frontendBackendTransferInternal(RendererBackend * loader, bool freeFrontendData = true);

    inline void * getMeshBackendData() const {
        return m_meshBackendData;
    }

    inline MeshData<> * getMeshFrontentData() const {
        return m_meshFrontendData;
    }

private:
    MeshData<> * m_meshFrontendData;
    RendererBackend * m_backend;
    void * m_meshBackendData;
};
}

#endif
