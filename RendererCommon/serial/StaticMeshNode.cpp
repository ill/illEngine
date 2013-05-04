#include <cassert>
#include "StaticMeshNode.h"
#include "GraphicsScene.h"

namespace illRendererCommon {

void StaticMeshNode::render(RenderQueues& renderQueues) {
    assert(!m_mesh.isNull());
    
    //place the mesh in the appropriate render queue
    for(uint8_t groupInd = 0; groupInd < m_primitiveGroups.size(); groupInd++) {
        PrimitiveGroupInfo& group = m_primitiveGroups[groupInd];

        assert(!group.m_material.isNull());

        switch(group.m_material->getLoadArgs().m_blendMode) {
        case illGraphics::MaterialLoadArgs::BlendMode::NONE: {
                {
                    MeshData<>::PrimitiveGroup& test = m_mesh->getMeshFrontentData()->getPrimitiveGroup(groupInd);

                    auto& list = renderQueues.m_depthPassSolidStaticMeshes[group.m_material->getDepthPassProgram()][group.m_material.get()][m_mesh.get()];

                    list.emplace_back();
                    list.back().m_node = this;
                    list.back().m_primitiveGroup = groupInd;
                }
                
                {
                    auto& list = renderQueues.m_solidStaticMeshes[group.m_material->getShaderProgram()][group.m_material.get()][m_mesh.get()];

                    list.emplace_back();
                    list.back().m_meshInfo.m_node = this;
                    list.back().m_meshInfo.m_primitiveGroup = groupInd;

                    if(renderQueues.m_getSolidAffectingLights || group.m_material->getLoadArgs().m_forceForwardRendering) {
                        getScene()->getLights(getWorldBoundingVolume(), list.back().m_affectingLights);
                    }
                }
            }
            break;

        case illGraphics::MaterialLoadArgs::BlendMode::ADDITIVE: {
                auto& list = renderQueues.m_unsolidStaticMeshes[group.m_material->getShaderProgram()][group.m_material.get()][m_mesh.get()];

                list.emplace_back();
                list.back().m_meshInfo.m_node = this;
                list.back().m_meshInfo.m_primitiveGroup = groupInd;

                //TODO: when this is used, also store whether or not an occlusion query is needed for the node

                if(renderQueues.m_getSolidAffectingLights || group.m_material->getLoadArgs().m_forceForwardRendering) {
                    getScene()->getLights(getWorldBoundingVolume(), list.back().m_affectingLights);
                }
            }
            break;

        case illGraphics::MaterialLoadArgs::BlendMode::ALPHA:
        case illGraphics::MaterialLoadArgs::BlendMode::PREMULT_ALPHA: {
                //TODO: handle the depth sorted stuff
            }
            break;
        }
    }
}

}