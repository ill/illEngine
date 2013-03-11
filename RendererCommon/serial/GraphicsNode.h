#ifndef ILL_GRAPHICS_ENTITY_H_
#define ILL_GRAPHICS_ENTITY_H_

#include "Util/Geometry/geomUtil.h"
#include "RendererCommon/serial/RenderQueues.h"

namespace illRendererCommon {

class GraphicsScene;

class GraphicsNode {
public:
    enum class State {
        IN_SCENE,
        OUT_SCENE
    };

    enum class Type {
        MESH,
        SKELETAL_MESH,
        LIGHT
    };

    virtual ~GraphicsNode() {
        if(m_state == State::IN_SCENE) {
            removeFromScene();
        }
    }

    virtual void render(RenderQueues& renderQueues, uint64_t frameCounter) = 0;

    inline glm::vec3 getPosition() const {
        return getTransformPosition(m_transform);
    }

    inline const glm::mat4& getTransform() const {
        return m_transform;
    }

    inline const Box<>& getBoundingVolume() const {
        return m_boundingVol;
    }

    inline Box<> getWorldBoundingVolume() const {
        return m_boundingVol + getPosition();
    }

    void move(const glm::vec3& position);
    void move(const glm::mat4& transform);
    void move(const Box<>& boundingVolume);

    void move(const glm::vec3& position, const Box<>& boundingVolume);
    void move(const glm::mat4& transform, const Box<>& boundingVolume);

    void addToScene();
    void removeFromScene();

    inline GraphicsScene * getScene() const {
        return m_scene;
    }
    
    inline State getState() const {
        return m_state;
    }

    inline Type getType() const {
        return m_type;
    }

protected:
    inline GraphicsNode(GraphicsScene * scene,
            const glm::mat4& transform, const Box<>& boundingVol,
            Type type, State initialState = State::IN_SCENE)
        : m_accessCounter(0),
        m_frameCounter(0),
        m_transform(transform),
        m_boundingVol(boundingVol),
        m_scene(scene),
        m_type(type)
    {
        if(initialState == State::IN_SCENE) {
            m_state = State::OUT_SCENE;
            addToScene();
        }
        else {
            m_state = initialState;
        }
    }

    /**
    This is so visibility culling queries don't return duplicate results when an object intersects multiple cells.
    */
    uint64_t m_frameCounter;

private:
    /**
    This is so scene queries don't return duplicate results when an object intersects multiple cells.
    */
    uint64_t m_accessCounter;

    glm::mat4 m_transform;

    /**
    The node's local bounding box not relative to the world.
    To find it relative to the world, offset it by the transform position.
    */
    Box<> m_boundingVol;

    GraphicsScene * m_scene;

    State m_state;
    Type m_type;

    friend class GraphicsScene;
};

}

#endif