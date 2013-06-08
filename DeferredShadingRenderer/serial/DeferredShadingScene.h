#ifndef ILL_DEFERRED_SHADING_SCENE_H_
#define ILL_DEFERRED_SHADING_SCENE_H_

#include <unordered_map>
#include "Util/serial/Array.h"
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
            cellDimensions, cellNumber, interactionCellDimensions, interactionCellNumber, true),
        m_frameCounter(30),
        m_maxQueries(4000),
        m_returnViewportId(0),
        m_queryVisibilityDuration(10),
        m_queryInvisibilityDuration(1),
        m_queryVisibilityDurationGrowth(1),
        m_queryInvisibilityDurationGrowth(1),
        m_numFramesOverflowed(0),
        m_performCull(true),
        m_debugPerObjectCull(false),
        
        m_debugMaxCellTraversals(-1)
    {
        m_renderQueues.m_queueLights = true;
        m_renderQueues.m_getSolidAffectingLights = false;
    }
    
    virtual ~DeferredShadingScene() {
    }

    virtual void setupFrame();

    virtual void render(const illGraphics::Camera& camera, size_t viewport, 
        MeshEdgeList<>* debugFrustum = NULL); //TODO: take out these debug things)

    /**
    For every main viewport you will use, you must register it first.
    This is so the scene can keep track of occlusion queries per viewport.

    If you have just one viewport just call this once.
    If you multiplayer split screen or something, call this for every viewport you will use.

    This will return to you a new viewport id and will allocate
    memory to hold data about occlusion queries.
    */
    size_t registerViewport();

    /**
    When no longer using a viewport, free it.
    */
    void freeViewport(size_t viewport);

    bool m_performCull;
    bool m_debugPerObjectCull;

    int m_debugNumTraversedCells;
    int m_debugNumQueries;
    int m_debugNumUnqueried;
    int m_debugNumEmptyCells;
    int m_debugNumCulledCells;
    int m_debugRequeryDuration;
    int m_debugNumRenderedNodes;
    int m_debugNumOverflowedQueries;

    int m_debugMaxCellTraversals;

protected:

    uint64_t m_frameCounter;

    uint64_t m_maxQueries;
    uint64_t m_queryVisibilityDuration;
    uint64_t m_queryInvisibilityDuration;
    uint64_t m_queryVisibilityDurationGrowth;
    uint64_t m_queryInvisibilityDurationGrowth;
    size_t m_numFramesOverflowed;

    size_t m_returnViewportId;  //the next viewport id that will be returned
    std::unordered_map<size_t, Array<uint64_t>> m_queryFrames;
};

}

#endif