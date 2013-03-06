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

    virtual void render(RenderQueues& renderQueues);

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

private:
    illGraphics::MeshId m_meshId;
    RefCountPtr<illGraphics::Mesh> m_mesh;

    illGraphics::MaterialId m_materialId;
    RefCountPtr<illGraphics::Material> m_material;
};

}

#endif