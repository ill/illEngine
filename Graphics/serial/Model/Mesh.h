#ifndef ILL_MESH_H__
#define ILL_MESH_H__

#include <string>
#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"
#include "Util/Geometry/MeshData.h"

namespace illGraphics {

class GraphicsBackend;


struct MeshLoadArgs {
    std::string m_path; //path of mesh file

    //TODO: more to come?  Maybe?
};

/**
Contains some mesh and its associated vertex buffer objects and index buffer objects.
*/
class Mesh : public ResourceBase<MeshLoadArgs, GraphicsBackend> {
public:
    Mesh()
        : m_meshFrontendData(NULL),
        m_meshBackendData(NULL)
    {}

    ~Mesh() {
        unload();
    }
    
    virtual void unload();
    virtual void reload(GraphicsBackend * backend);

    void setFrontentDataInternal(MeshData<> * mesh);
    void frontendBackendTransferInternal(GraphicsBackend * loader, bool freeFrontendData = true);

    inline void * getMeshBackendData() const {
        return m_meshBackendData;
    }

    inline MeshData<> * getMeshFrontentData() const {
        return m_meshFrontendData;
    }

private:
    MeshData<> * m_meshFrontendData;
    GraphicsBackend * m_backend;
    void * m_meshBackendData;
};

typedef uint32_t MeshId;
typedef ConfigurableResourceManager<MeshId, Mesh, MeshLoadArgs, GraphicsBackend> MeshManager;
}

#endif
