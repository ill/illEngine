#include "DeferredShadingRenderer/serial/DeferredShadingScene.h"
#include "DeferredShadingRenderer/DeferredShadingBackend.h"
#include "Util/Geometry/ConvexMeshIterator.h"
#include "Graphics/serial/Camera/Camera.h"

namespace illDeferredShadingRenderer {

void DeferredShadingScene::render(const illGraphics::Camera& camera) {
    static_cast<DeferredShadingBackend *>(m_rendererBackend)->setupFrame();
    static_cast<DeferredShadingBackend *>(m_rendererBackend)->setupViewport(camera);

    //get the frustum iterator
    MeshEdgeList<> meshEdgeList = camera.getViewFrustum().getMeshEdgeList();
    ConvexMeshIterator<> frustumIterator = getGridVolume().meshIteratorForMesh(&meshEdgeList, camera.getViewFrustum().m_direction);

    while(!frustumIterator.atEnd()) {
        unsigned int currentCell = getGridVolume().indexForCell(frustumIterator.getCurrentPosition());
        frustumIterator.forward();

        //TODO: cell querying stuff
        
        //add all nodes in the cell to the render queues
        {
            auto& currCell = getSceneNodeCell(currentCell);

            for(auto cellIter = currCell.begin(); cellIter != currCell.end(); cellIter++) {
                (*cellIter)->render(m_renderQueues, m_frameCounter);
            }
        }

        {
            auto& currCell = getStaticNodeCell(currentCell);

            for(size_t arrayInd = 0; arrayInd < currCell.size(); arrayInd++) {
                currCell[arrayInd]->render(m_renderQueues, m_frameCounter);
            }
        }

        //draw objects for the depth pass
        static_cast<DeferredShadingBackend *>(m_rendererBackend)->depthPass(m_renderQueues, camera);
    }

    static_cast<DeferredShadingBackend *>(m_rendererBackend)->render(m_renderQueues, camera);

    ++m_frameCounter;

    m_renderQueues.m_depthPassSolidStaticMeshes.clear();
    m_renderQueues.m_lights.clear();
    m_renderQueues.m_solidStaticMeshes.clear();
}

}