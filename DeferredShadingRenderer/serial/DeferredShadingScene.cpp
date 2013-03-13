#include <new>

#include "DeferredShadingRenderer/serial/DeferredShadingScene.h"
#include "DeferredShadingRenderer/DeferredShadingBackend.h"
#include "Util/Geometry/ConvexMeshIterator.h"
#include "Graphics/serial/Camera/Camera.h"

namespace illDeferredShadingRenderer {

void DeferredShadingScene::setupFrame() {
    static_cast<DeferredShadingBackend *>(m_rendererBackend)->retreiveCellQueries(m_lastVisibleFrames, m_frameCounter);
    static_cast<DeferredShadingBackend *>(m_rendererBackend)->setupFrame();
    ++m_frameCounter;
}

void DeferredShadingScene::render(const illGraphics::Camera& camera, size_t viewport) {    
    static_cast<DeferredShadingBackend *>(m_rendererBackend)->setupViewport(camera);

    //get the frustum iterator
    MeshEdgeList<> meshEdgeList = camera.getViewFrustum().getMeshEdgeList();
    ConvexMeshIterator<> frustumIterator = getGridVolume().meshIteratorForMesh(&meshEdgeList, camera.getViewFrustum().m_direction);

    while(!frustumIterator.atEnd()) {
        unsigned int currentCell = getGridVolume().indexForCell(frustumIterator.getCurrentPosition());

        //do an occlusion query for the cell
        void * cellQuery = static_cast<DeferredShadingBackend *>(m_rendererBackend)->occlusionQueryCell(
            camera, vec3cast<unsigned int, glm::mediump_float>(frustumIterator.getCurrentPosition()) * getGridVolume().getCellDimensions() + getGridVolume().getCellDimensions() * 0.5f, 
            getGridVolume().getCellDimensions(), currentCell, viewport);

        frustumIterator.forward();

        //if cell was visible last frame
        if(m_lastVisibleFrames.at(viewport)[currentCell] == m_frameCounter - 1 || !m_performCull) {
            //add all nodes in the cell to the render queues
            {
                auto& currCell = getSceneNodeCell(currentCell);

                for(auto cellIter = currCell.begin(); cellIter != currCell.end(); cellIter++) {
                    (*cellIter)->render(m_renderQueues, m_renderAccessCounter);
                }
            }

            {
                auto& currCell = getStaticNodeCell(currentCell);

                for(size_t arrayInd = 0; arrayInd < currCell.size(); arrayInd++) {
                    currCell[arrayInd]->render(m_renderQueues, m_renderAccessCounter);
                }
            }

            //draw objects for the depth pass
            static_cast<DeferredShadingBackend *>(m_rendererBackend)->depthPass(m_renderQueues, camera, cellQuery);
        }
    }

    static_cast<DeferredShadingBackend *>(m_rendererBackend)->render(m_renderQueues, camera);

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