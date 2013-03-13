#ifndef ILL_STATIC_MESH_NODE_H_
#define ILL_STATIC_MESH_NODE_H_

#include "RendererCommon/serial/GraphicsNode.h"
#include "Util/serial/RefCountPtr.h"
#include "Graphics/serial/Model/Mesh.h"
#include "Graphics/serial/Material/Material.h"

namespace illRendererCommon {

class StaticMeshNode : public GraphicsNode {
public:
    inline StaticMeshNode(GraphicsScene * scene,
            illGraphics::MeshId meshId,
            illGraphics::MaterialId materialId,
            const glm::mat4& transform, const Box<>& boundingVol,
            State initialState = State::IN_SCENE)
        : GraphicsNode(scene, transform, boundingVol, Type::MESH, initialState),
        m_meshId(meshId),
        m_materialId(meshId)
    {}

    virtual ~StaticMeshNode() {}

    virtual void render(RenderQueues& renderQueues, uint64_t renderAccessCounter);

    inline void load(illGraphics::MeshManager * meshManager, illGraphics::MaterialManager * materialManager) {
        if(m_mesh.isNull()) {
            m_mesh = meshManager->getResource(m_meshId);
        }

        if(m_material.isNull()) {
            m_material = materialManager->getResource(m_materialId);
        }
    }

    inline void unload() {
        m_mesh.reset();
    }

    inline illGraphics::MeshId getMeshId() const {
        return m_meshId;
    }

    inline RefCountPtr<illGraphics::Mesh> getMeshReference() const {
        return m_mesh;
    }

    inline const illGraphics::Mesh * getMesh() const {
        return m_mesh.get();
    }

    inline illGraphics::MaterialId getMaterialId() const {
        return m_materialId;
    }

    inline RefCountPtr<illGraphics::Material> getMaterialReference() const {
        return m_material;
    }

    inline const illGraphics::Material * getMaterial() const {
        return m_material.get();
    }

private:
    illGraphics::MeshId m_meshId;
    RefCountPtr<illGraphics::Mesh> m_mesh;

    illGraphics::MaterialId m_materialId;
    RefCountPtr<illGraphics::Material> m_material;
};

}

#endif