#ifndef ILL_LIGHT_NODE_H_
#define ILL_LIGHT_NODE_H_

#include <cassert>
#include "RendererCommon/serial/GraphicsNode.h"
#include "Graphics/serial/Light.h"

namespace illRendererCommon {
class LightNode : public GraphicsNode {
public:
    inline LightNode(GraphicsScene * scene,
            illGraphics::LightBase * light,
            const glm::mat4& transform, const Box<>& boundingVol,
            State initialState = State::IN_SCENE)
        : GraphicsNode(scene, transform, boundingVol, Type::LIGHT, initialState),
        m_light(light)
    {
        assert(m_light);
    }

    virtual ~LightNode() {
        delete m_light;
    }

    virtual void render(RenderQueues& renderQueues);

private:
    illGraphics::LightBase * m_light;
};
}

#endif