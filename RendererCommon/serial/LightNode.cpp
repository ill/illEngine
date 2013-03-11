#include "LightNode.h"

namespace illRendererCommon {

void LightNode::render(RenderQueues& renderQueues, uint64_t frameCounter) {    
    //avoid duplicates
    if(m_frameCounter <= frameCounter) {
        m_frameCounter = frameCounter + 1;
    }
    else {
        return;
    }
    
    if(renderQueues.m_queueLights) {
        renderQueues.m_lights[m_light->getType()][m_light].push_back(this);
    }
}

}