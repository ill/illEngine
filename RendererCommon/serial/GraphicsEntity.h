#ifndef ILL_GRAPHICS_ENTITY_H_
#define ILL_GRAPHICS_ENTITY_H_

#include "Util/Geometry/SceneNode.h"

namespace illRendererCommon {

class RenderArgs;
class GraphicsScene;

class GraphicsEntity {
public:
    //TODO: this entity and node pattern looks like it'll be usable for other scene types, redesign it later so I can use
    //this for the audio scene and stuff as well
    class GraphicsNode : public SceneNode<> {
    public:
        enum class State {
            IN_SCENE,
            OUT_SCENE
        };

        enum class Type {
            GENERIC,
            LIGHT
        };

        inline GraphicsNode(GraphicsEntity * parent, GraphicsScene * scene, Type type = Type::GENERIC, State initialState = State::IN_SCENE)
            : m_parent(parent),
            m_scene(scene),
            m_type(type)
        {
            if(initialState == State::IN_SCENE) {
                m_state = State::OUT_SCENE;
                rootAddToScene();
            }
            else {
                m_state = initialState;
            }
        }

        inline virtual ~GraphicsNode() {
            if(getParent() == NULL) {
                rootRemoveFromScene();
            }
        }

        inline void render(RenderArgs& dest) const {
            m_parent->render(dest);
        }

        void rootMove(const glm::vec3& position);
        void rootMove(const glm::mat4& transform);
        void rootMove(const Box<>& boundingBox);

        void rootMove(const glm::vec3& position, const Box<>& boundingBox);
        void rootMove(const glm::mat4& transform, const Box<>& boundingBox);

        void rootAddToScene();
        void rootRemoveFromScene();

        void childMove(const glm::mat4& relativeTransform);
        void childMove(const Box<>& boundingBox);
        void childMove(const glm::mat4& relativeTransform, const Box<>& boundingBox);

        inline void addChild(GraphicsNode * node) {
            if(node->getState() == State::IN_SCENE) {
                rootRemoveFromScene();
            }

            SceneNode::addChild(node);
        }

        inline void removeChild(GraphicsNode * node) {
            SceneNode::removeChild(node);
        }

        inline State getState() const {
            return m_state;
        }

        inline Type getType() const {
            return m_type;
        }

    private:
        GraphicsEntity * m_parent;
        GraphicsScene * m_scene;

        State m_state;
        Type m_type;

        friend class GraphicsScene;
    };

    virtual ~GraphicsEntity() {}

protected:
    GraphicsEntity() {}

    virtual void render(RenderArgs& dest) = 0;
};

}

#endif