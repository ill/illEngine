#include <new>

#include "DeferredShadingRenderer/serial/DeferredShadingScene.h"
#include "DeferredShadingRenderer/DeferredShadingBackend.h"
#include "Util/Geometry/Iterators/MultiConvexMeshIterator.h"
#include "Graphics/serial/Camera/Camera.h"

namespace illDeferredShadingRenderer {

void DeferredShadingScene::setupFrame() {
    static_cast<DeferredShadingBackend *>(m_rendererBackend)->retreiveCellQueries(m_lastVisibleFrames, m_frameCounter);
    static_cast<DeferredShadingBackend *>(m_rendererBackend)->retreiveNodeQueries(m_frameCounter);
    static_cast<DeferredShadingBackend *>(m_rendererBackend)->setupFrame();
    ++m_frameCounter;
}

void DeferredShadingScene::render(const illGraphics::Camera& camera, size_t viewport) {    
    static_cast<DeferredShadingBackend *>(m_rendererBackend)->setupViewport(camera);

    //get the frustum iterator
    MeshEdgeList<> meshEdgeList = camera.getViewFrustum().getMeshEdgeList();
    MultiConvexMeshIterator<> frustumIterator;
    
    getGridVolume().orderedMeshIteratorForMesh(frustumIterator, &meshEdgeList,
        camera.getViewFrustum().m_nearTipPoint,
        camera.getViewFrustum().m_direction);

    bool needsQuerySetup = true;

    while(!frustumIterator.atEnd()) {
        unsigned int currentCell = getGridVolume().indexForCell(frustumIterator.getCurrentPosition());

        //check if cell is empty
        if(getSceneNodeCell(currentCell).empty() && getStaticNodeCell(currentCell).size() == 0) {
            frustumIterator.forward();
            continue;
        }

        //do an occlusion query for the cell
        if(needsQuerySetup) {
            static_cast<DeferredShadingBackend *>(m_rendererBackend)->setupQuery();
            needsQuerySetup = false;
        }

        void * cellQuery = static_cast<DeferredShadingBackend *>(m_rendererBackend)->occlusionQueryCell(
            camera, vec3cast<unsigned int, glm::mediump_float>(frustumIterator.getCurrentPosition()) * getGridVolume().getCellDimensions() 
                + getGridVolume().getCellDimensions() * 0.5f, 
            getGridVolume().getCellDimensions(), currentCell, viewport);

        frustumIterator.forward();

        //if cell was visible last frame and has objects in it
        if(m_lastVisibleFrames.at(viewport)[currentCell] == m_frameCounter - 1 || !m_performCull) {
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
                    }
                }
            }

            static_cast<DeferredShadingBackend *>(m_rendererBackend)->endQuery();
            needsQuerySetup = true;

            //draw objects for the depth pass
            static_cast<DeferredShadingBackend *>(m_rendererBackend)->depthPass(m_renderQueues, camera, cellQuery, viewport);
        }
    }

    static_cast<DeferredShadingBackend *>(m_rendererBackend)->render(m_renderQueues, camera, viewport);

    ++m_renderAccessCounter;

    m_renderQueues.m_depthPassSolidStaticMeshes.clear();
    m_renderQueues.m_lights.clear();
    m_renderQueues.m_solidStaticMeshes.clear();
}

size_t DeferredShadingScene::registerViewport() {
    size_t res = m_returnViewportId++;
    Array<uint64_t>& framesArray = m_lastVisibleFrames[res];

    size_t numCells = getGridVolume().getCellNumber().x * getGridVolume().getCellNumber().y * getGridVolume().getCellNumber().z;

    framesArray.resize(numCells);
    memset(&framesArray[0], 0, sizeof(uint64_t) * numCells);

    return res;
}

void DeferredShadingScene::freeViewport(size_t viewport) {
    m_lastVisibleFrames.erase(viewport);
}

}