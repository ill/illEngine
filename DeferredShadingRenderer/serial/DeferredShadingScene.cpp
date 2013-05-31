#include "DeferredShadingRenderer/serial/DeferredShadingScene.h"
#include "DeferredShadingRenderer/DeferredShadingBackend.h"
#include "Util/Geometry/Iterators/MultiConvexMeshIterator.h"
#include "Graphics/serial/Camera/Camera.h"

namespace illDeferredShadingRenderer {

void DeferredShadingScene::setupFrame() {
    uint64_t visibilityDuration = m_queryVisibilityDuration;
    uint64_t invisibilityDuration = m_queryInvisibilityDuration;

    if(m_numFramesOverflowed > 0) {
        visibilityDuration += m_queryVisibilityDurationGrowth * m_numFramesOverflowed;
        invisibilityDuration += m_queryInvisibilityDurationGrowth * m_numFramesOverflowed;
    }

    static_cast<DeferredShadingBackend *>(m_rendererBackend)->retreiveCellQueries(m_queryFrames, m_frameCounter, 
        //1, 1, 0);     //to test missing cells
        visibilityDuration, invisibilityDuration, m_numFramesOverflowed * 2);
    static_cast<DeferredShadingBackend *>(m_rendererBackend)->retreiveNodeQueries(m_frameCounter);
    static_cast<DeferredShadingBackend *>(m_rendererBackend)->setupFrame();
   
    ++m_frameCounter;
}

void DeferredShadingScene::render(const illGraphics::Camera& camera, size_t viewport, 
        MeshEdgeList<>* debugFrustum) {
    m_renderQueues.m_depthPassObjects = 75;
    static_cast<DeferredShadingBackend *>(m_rendererBackend)->setupViewport(camera);

    //get the frustum iterator
    MeshEdgeList<> meshEdgeList = camera.getViewFrustum().getMeshEdgeList();
    MultiConvexMeshIterator<> frustumIterator;
    
    getGridVolume().orderedMeshIteratorForMesh(frustumIterator, &meshEdgeList,
        camera.getViewFrustum().m_nearTipPoint,
        camera.getViewFrustum().m_direction);

    if(m_debugCapturingFrustumIter) {
        m_debugCapturingFrustumIter = false;
        //delete m_debugBackupFrustumIterator;
        //delete m_debugFrustumIterator;

        m_debugFrustumTraversals.clear();

        m_debugBackupFrustumIterator = new MultiConvexMeshIterator<>(frustumIterator);
        m_debugFrustumIterator = new MultiConvexMeshIterator<>(frustumIterator);

        if(!m_debugFrustumIterator->atEnd()) {
            m_debugFrustumTraversals.push_back(m_debugFrustumIterator->getCurrentPosition());
        }
    }

    bool needsQuerySetup = true;

    m_debugNumTraversedCells = 0;
    m_debugNumEmptyCells = 0;
    m_debugNumCulledCells = 0;
    m_debugNumRenderedNodes = 0;
    m_debugNumUnqueried = 0;
    
    size_t numQueries = 0;
    size_t numQueriesNeeded = 0;
    
    //std::set<unsigned int> debugCellSet;
    
    bool recordedOverflow = false;

    while(!frustumIterator.atEnd()) {
        unsigned int currentCell = getGridVolume().indexForCell(frustumIterator.getCurrentPosition());

        /*if(debugCellSet.find(currentCell) != debugCellSet.end()) {
            LOG_ERROR("Cell %u traversed multiple times", currentCell);
        }

        debugCellSet.insert(currentCell);*/

        ++m_debugNumTraversedCells;

        //check if cell is empty
        if(getSceneNodeCell(currentCell).empty() && getStaticNodeCell(currentCell).size() == 0) {
            ++m_debugNumEmptyCells;
            frustumIterator.forward();
            continue;
        }

        ++numQueriesNeeded;

        //do an occlusion query for the cell
        void * cellQuery = NULL;
        
        bool visible = DeferredShadingBackend::decodeVisible(m_queryFrames.at(viewport)[currentCell]);
        uint64_t lastQueryFrame = DeferredShadingBackend::codeFrame(m_queryFrames.at(viewport)[currentCell]);
                
        //time to query
        if(lastQueryFrame <= m_frameCounter) {
            if(numQueries >= m_maxQueries) {  //check if queries overflowed
                if(!recordedOverflow) {
                    ++m_numFramesOverflowed;
                    recordedOverflow = true;
                }
            }

            if(numQueries >= m_maxQueries && !visible) {  //if queries overflowed, force visible cells to requery to avoid blinking
                ++m_debugNumUnqueried;
            }
            else {
                ++numQueries;

                if(needsQuerySetup) {
                    static_cast<DeferredShadingBackend *>(m_rendererBackend)->setupQuery();
                    needsQuerySetup = false;
                }

                cellQuery = static_cast<DeferredShadingBackend *>(m_rendererBackend)->occlusionQueryCell(
                    camera, vec3cast<unsigned int, glm::mediump_float>(frustumIterator.getCurrentPosition()) * getGridVolume().getCellDimensions() 
                        + getGridVolume().getCellDimensions() * 0.5f, 
                    getGridVolume().getCellDimensions(), currentCell, viewport);
            }
        }
        else {
            ++m_debugNumUnqueried;
        }
        
        frustumIterator.forward();

        //if cell was visible last frames and has objects in it
        if((visible && lastQueryFrame >= m_frameCounter) || !m_performCull) {
            //add all nodes in the cell to the render queues
            {
                auto& currCell = getSceneNodeCell(currentCell);

                for(auto cellIter = currCell.begin(); cellIter != currCell.end(); cellIter++) {
                    auto node = *cellIter;

                    if(node->addedToRenderQueue(m_renderAccessCounter)) {
                        continue;
                    }

                    //TODO: take this out after done with thesis                    
                    node->setOcclusionCull(m_debugPerObjectCull);

                    if(node->getOcclusionCull() && node->getLastNonvisibleFrame(viewport) == m_frameCounter - 1) {
                        static_cast<DeferredShadingBackend *>(m_rendererBackend)->occlusionQueryNode(camera, node, viewport);
                    }
                    else {
                        node->render(m_renderQueues);
                        ++m_debugNumRenderedNodes;
                    }
                }
            }

            {
                auto& currCell = getStaticNodeCell(currentCell);

                for(size_t arrayInd = 0; arrayInd < currCell.size(); arrayInd++) {
                    auto node = currCell[arrayInd];

                    if(node->addedToRenderQueue(m_renderAccessCounter)) {
                        continue;
                    }

                    //TODO: take this out after done with thesis                    
                    node->setOcclusionCull(m_debugPerObjectCull);

                    if(node->getOcclusionCull() && node->getLastNonvisibleFrame(viewport) == m_frameCounter - 1) {
                        static_cast<DeferredShadingBackend *>(m_rendererBackend)->occlusionQueryNode(camera, node, viewport);
                    }
                    else {
                        node->render(m_renderQueues);
                        ++m_debugNumRenderedNodes;
                    }
                }
            }

            static_cast<DeferredShadingBackend *>(m_rendererBackend)->endQuery();
            needsQuerySetup = true;

            //draw objects for the depth pass
            static_cast<DeferredShadingBackend *>(m_rendererBackend)->depthPass(m_renderQueues, camera, cellQuery, viewport);
        }
        else {
            ++m_debugNumCulledCells;
        }
    }

    if(numQueries < m_maxQueries) {        
        m_numFramesOverflowed = 0;
    }

    //LOG_DEBUG("Num Queries needed %u.  Countdown %u.  Num performed %u", numQueriesNeeded, m_resetRequeryDurationCountdown, numQueries);

    m_debugNumOverflowedQueries = m_numFramesOverflowed;
    m_debugRequeryDuration = m_queryVisibilityDuration;    
    m_debugNumQueries = numQueries;

    static_cast<DeferredShadingBackend *>(m_rendererBackend)->render(m_renderQueues, camera, viewport, 
        &m_grid, debugFrustum, &m_queryFrames, m_frameCounter, m_debugFrustumIterator, &m_debugFrustumTraversals);

    ++m_renderAccessCounter;

    m_renderQueues.m_depthPassSolidStaticMeshes.clear();
    m_renderQueues.m_lights.clear();
    m_renderQueues.m_solidStaticMeshes.clear();
    m_renderQueues.m_depthPassObjects = 0;
}

size_t DeferredShadingScene::registerViewport() {
    size_t res = m_returnViewportId++;
    Array<uint64_t>& framesArray = m_queryFrames[res];

    size_t numCells = getGridVolume().getCellNumber().x * getGridVolume().getCellNumber().y * getGridVolume().getCellNumber().z;

    framesArray.resize(numCells);
    memset(&framesArray[0], 0, sizeof(uint64_t) * numCells);

    return res;
}

void DeferredShadingScene::freeViewport(size_t viewport) {
    m_queryFrames.erase(viewport);
}

}