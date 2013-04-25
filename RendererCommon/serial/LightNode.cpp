#include "LightNode.h"
#include <glm/gtc/random.hpp>

namespace illRendererCommon {

void LightNode::render(RenderQueues& renderQueues) {
    //Uncomment for horrible disco mode
    //m_light->m_color = glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f));

    if(renderQueues.m_queueLights) {
        renderQueues.m_lights[m_light->getType()][m_light.get()].emplace_back(this);
    }
}

}