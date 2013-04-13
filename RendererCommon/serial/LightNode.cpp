#include "LightNode.h"

namespace illRendererCommon {

void LightNode::render(RenderQueues& renderQueues, uint64_t renderAccessCounter) {    
    //avoid duplicates
    if(m_renderAccessCounter <= renderAccessCounter) {
        m_renderAccessCounter = renderAccessCounter + 1;
    }
    else {
        return;
    }
    
    if(renderQueues.m_queueLights) {
        auto& list = renderQueues.m_lights[m_light->getType()][m_light];

        list.emplace_back();
        list.back().m_light = this;
        list.back().m_useOcclusionQuery = getOcclusionCull();
    }
}

}