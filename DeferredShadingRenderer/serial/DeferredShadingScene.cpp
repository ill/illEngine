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

    illRendererCommon::RenderQueues renderQueues;
    renderQueues.m_queueLights = true;
    renderQueues.m_getSolidAffectingLights = false;
    
    while(!frustumIterator.atEnd()) {
        unsigned int currentCell = getGridVolume().indexForCell(frustumIterator.getCurrentPosition());
        frustumIterator.forward();

        //TODO: cell querying stuff
        
        //add all nodes in the cell to the render queues
        {
            auto& currCell = getSceneNodeCell(currentCell);

            for(auto cellIter = currCell.begin(); cellIter != currCell.end(); cellIter++) {
                (*cellIter)->render(renderQueues, m_frameCounter);
            }
        }

        {
            auto& currCell = getStaticNodeCell(currentCell);

            for(size_t arrayInd = 0; arrayInd < currCell.size(); arrayInd++) {
                currCell[arrayInd]->render(renderQueues, m_frameCounter);
            }
        }

        //draw objects for the depth pass
        static_cast<DeferredShadingBackend *>(m_rendererBackend)->depthPass(renderQueues, camera);
    }

    static_cast<DeferredShadingBackend *>(m_rendererBackend)->render(renderQueues, camera);

    ++m_frameCounter;
}

}