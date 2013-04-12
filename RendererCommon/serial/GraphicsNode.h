#ifndef ILL_GRAPHICS_ENTITY_H_
#define ILL_GRAPHICS_ENTITY_H_

#include <unordered_map>
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

    /**
    Invokes the node to add itself to the render queue.  It's up to the node to do this correctly
    and add itself into the right queue.

    @param renderQueues The render queues struct that has all the objects to render queued up.
    @param renderAccessCounter A counter for helping to prevent duplicates added if a node intersects cell boundaries.
    */
    virtual void render(RenderQueues& renderQueues, uint64_t renderAccessCounter) = 0;

    /**
    Gets the world position of the node in the scene.
    */
    inline glm::vec3 getPosition() const {
        return getTransformPosition(m_transform);
    }

    /**
    Gets the world transform of the node in the scene.  Use this for rendering.
    */
    inline const glm::mat4& getTransform() const {
        return m_transform;
    }

    /**
    Gets the local bounding volume around the origin.  This should encompass the entire object as tightly as possible.
    */
    inline const Box<>& getBoundingVolume() const {
        return m_boundingVol;
    }

    /**
    Gets the world bounding volume by just taking the bounding volume and offsetting it by the world position.
    This determines a lot of things like the scene grid cells where the node will be stored, and possibly some
    broadphase collision checks or something.
    */
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

    inline void setOcclusionCull(bool occlusionCull) {
        m_occlusionCull = occlusionCull;
    }

    inline bool getOcclusionCull() const {
        return m_occlusionCull;
    }

    inline void setLastVisibleFrame(size_t viewport, uint64_t frame) {
        m_lastVisibleFrames[viewport] = frame;
    }

    inline uint64_t getLastVisibleFrame(size_t viewport) const {
        auto iter = m_lastVisibleFrames.find(viewport);

        if(iter == m_lastVisibleFrames.end()) {
            return 0;
        }
        else {
            return iter->second;
        }
    }

protected:
    inline GraphicsNode(GraphicsScene * scene,
            const glm::mat4& transform, const Box<>& boundingVol,
            Type type, State initialState = State::IN_SCENE)
        : m_accessCounter(0),
        m_renderAccessCounter(0),
        m_transform(transform),
        m_boundingVol(boundingVol),
        m_scene(scene),
        m_type(type),
        m_occlusionCull(false)
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
    uint64_t m_renderAccessCounter;

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

    /**
    Whether or not occlusion queries should be used for this node along with the cell queries the 
    renderer normally performs.  This has no effect if using a renderer that doesn't perform
    occlusion queries.
    */
    bool m_occlusionCull;

    /**
    If the node uses occlusion queries, this is a map of viewport to the last frame this node
    passed an occlusion query.
    */
    std::unordered_map<size_t, uint64_t> m_lastVisibleFrames;

    friend class GraphicsScene;
};

}

#endif