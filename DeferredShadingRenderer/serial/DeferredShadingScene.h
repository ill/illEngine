#ifndef ILL_DEFERRED_SHADING_SCENE_H_
#define ILL_DEFERRED_SHADING_SCENE_H_

#include <unordered_set>
#include <cstring>

#include "Util/Geometry/GridVolume3D.h"
#include "RendererCommon/serial/GraphicsScene.h"

template <typename T = glm::mediump_float>
struct Frustum;

namespace illDeferredShadingRenderer {

class DeferredShadingScene : public illRendererCommon::GraphicsScene {
public:
    inline DeferredShadingScene(illRendererCommon::RendererBackend * rendererBackend, 
            glm::vec3& cellDimensions, const glm::uvec3& cellNumber,
            glm::vec3& interactionCellDimensions, const glm::uvec3& interactionCellNumber)
        : GraphicsScene(rendererBackend, cellDimensions, cellNumber, interactionCellDimensions, interactionCellNumber, true),
        m_currentFrame(1) {
        m_lastVisibleFrame = new uint64_t[cellNumber.x * cellNumber.y * cellNumber.z];

        memset(m_lastVisibleFrame, 0, sizeof(uint64_t));
    }
    
    virtual ~DeferredShadingScene() {
        delete[] m_lastVisibleFrame;
    }

    virtual void render(const illGraphics::Camera& camera);

protected:
    uint64_t m_currentFrame;

    /**
    For each cell, what was the last frame that this cell was visible
    */
    uint64_t * m_lastVisibleFrame;
};

}

#endif