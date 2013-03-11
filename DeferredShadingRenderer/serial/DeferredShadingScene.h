#ifndef ILL_DEFERRED_SHADING_SCENE_H_
#define ILL_DEFERRED_SHADING_SCENE_H_

#include <unordered_set>
#include <cstring>

#include "Util/Geometry/GridVolume3D.h"
#include "RendererCommon/serial/GraphicsScene.h"

#include "DeferredShadingRenderer/DeferredShadingBackend.h"

namespace illDeferredShadingRenderer {

class DeferredShadingScene : public illRendererCommon::GraphicsScene {
public:
    inline DeferredShadingScene(DeferredShadingBackend * rendererBackend,
            illGraphics::MeshManager * meshManager, illGraphics::MaterialManager * materialManager,
            glm::vec3& cellDimensions, const glm::uvec3& cellNumber,
            glm::vec3& interactionCellDimensions, const glm::uvec3& interactionCellNumber)
        : GraphicsScene(rendererBackend, 
            meshManager, materialManager, 
            cellDimensions, cellNumber, interactionCellDimensions, interactionCellNumber, true)
    {
        m_lastVisibleFrame = new uint64_t[cellNumber.x * cellNumber.y * cellNumber.z];

        memset(m_lastVisibleFrame, 0, sizeof(uint64_t) * cellNumber.x * cellNumber.y * cellNumber.z);

        m_renderQueues.m_queueLights = true;
        m_renderQueues.m_getSolidAffectingLights = false;
    }
    
    virtual ~DeferredShadingScene() {
        delete[] m_lastVisibleFrame;
    }

    virtual void render(const illGraphics::Camera& camera);

protected:
    /**
    For each cell, what was the last frame that this cell was visible
    */
    uint64_t * m_lastVisibleFrame;
};

}

#endif