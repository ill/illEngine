#include <cassert>
#include "StaticMeshNode.h"
#include "GraphicsScene.h"

namespace illRendererCommon {

void StaticMeshNode::render(RenderQueues& renderQueues, uint64_t frameCounter) {
    //avoid duplicates
    if(m_frameCounter <= frameCounter) {
        m_frameCounter = frameCounter + 1;
    }
    else {
        return;
    }

    assert(!m_mesh.isNull());
    assert(!m_material.isNull());
    
    //place the mesh in the appropriate render queue
    switch(m_material->getLoadArgs().m_blendMode) {
    case illGraphics::MaterialLoadArgs::BlendMode::NONE: {
            renderQueues.m_depthPassSolidStaticMeshes[m_material->getDepthPassProgram()][m_material.get()][m_mesh.get()].emplace_back(this);

            {
                auto& list = renderQueues.m_solidStaticMeshes[m_material->getShaderProgram()][m_material.get()][m_mesh.get()];

                list.emplace_back();
                list.back().m_node = this;

                if(renderQueues.m_getSolidAffectingLights || m_material->getLoadArgs().m_forceForwardRendering) {
                    getScene()->getLights(getWorldBoundingVolume(), list.back().m_affectingLights);
                }
            }
        }
        break;

    case illGraphics::MaterialLoadArgs::BlendMode::ADDITIVE: {
            auto& list = renderQueues.m_unsolidStaticMeshes[m_material->getShaderProgram()][m_material.get()][m_mesh.get()];

            list.emplace_back();
            list.back().m_node = this;

            if(renderQueues.m_getSolidAffectingLights || m_material->getLoadArgs().m_forceForwardRendering) {
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