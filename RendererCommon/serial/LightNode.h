#ifndef ILL_LIGHT_NODE_H_
#define ILL_LIGHT_NODE_H_

#include <cassert>
#include "RendererCommon/serial/GraphicsNode.h"
#include "Graphics/serial/Light.h"
#include "Util/serial/RefCountPtr.h"

namespace illRendererCommon {
class LightNode : public GraphicsNode {
public:
    inline LightNode(GraphicsScene * scene,
            const glm::mat4& transform, const Box<>& boundingVol,
            State initialState = State::IN_SCENE)
        : GraphicsNode(scene, transform, boundingVol, Type::LIGHT, initialState)
    {}

    virtual ~LightNode() {}

    virtual void render(RenderQueues& renderQueues);

    RefCountPtr<illGraphics::LightBase> m_light;
};
}

#endif