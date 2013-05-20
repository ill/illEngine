#ifndef ILL_STATIC_MESH_NODE_H_
#define ILL_STATIC_MESH_NODE_H_

#include "RendererCommon/serial/GraphicsNode.h"
#include "Util/serial/RefCountPtr.h"
#include "Graphics/serial/Model/Mesh.h"
#include "Graphics/serial/Material/Material.h"

namespace illRendererCommon {

class StaticMeshNode : public GraphicsNode {
public:
    enum class OccluderType {
        ALWAYS,
        LIMITED,
        NEVER
    };

    inline StaticMeshNode(GraphicsScene * scene,
            const glm::mat4& transform, const Box<>& boundingVol,
            OccluderType occluderType = OccluderType::ALWAYS,
            State initialState = State::IN_SCENE)
        : GraphicsNode(scene, transform, boundingVol, Type::MESH, initialState),
        m_meshId(-1),
        m_occluderType(occluderType)
    {}

    virtual ~StaticMeshNode() {}

    virtual void render(RenderQueues& renderQueues);

    inline void load(illGraphics::MeshManager * meshManager, illGraphics::MaterialManager * materialManager) {
        if(m_mesh.isNull()) {
            m_mesh = meshManager->getResource(m_meshId);
        }
        
        for(auto iter = m_primitiveGroups.begin(); iter != m_primitiveGroups.end(); iter++) {
            PrimitiveGroupInfo& groupInfo = *iter;

            if(groupInfo.m_visible && groupInfo.m_material.isNull()) {
                groupInfo.m_material = materialManager->getResource(groupInfo.m_materialId);
            }
        }
    }

    inline void unload() {
        m_mesh.reset();

        for(auto iter = m_primitiveGroups.begin(); iter != m_primitiveGroups.end(); iter++) {
            iter->m_material.reset();
        }
    }
        
    illGraphics::MeshId m_meshId;
    RefCountPtr<illGraphics::Mesh> m_mesh;

    struct PrimitiveGroupInfo {
        PrimitiveGroupInfo()
            : m_materialId(-1),
            m_visible(false)
        {}

        illGraphics::MaterialId m_materialId;
        RefCountPtr<illGraphics::Material> m_material;
        bool m_visible;
    };

    OccluderType m_occluderType;
    std::vector<PrimitiveGroupInfo> m_primitiveGroups;
};

}

#endif