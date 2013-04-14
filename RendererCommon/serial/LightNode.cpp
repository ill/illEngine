#include "LightNode.h"

namespace illRendererCommon {

void LightNode::render(RenderQueues& renderQueues) {    
    if(renderQueues.m_queueLights) {
        renderQueues.m_lights[m_light->getType()][m_light].emplace_back(this);
    }
}

}