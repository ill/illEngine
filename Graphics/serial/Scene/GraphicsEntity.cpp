#include <cassert>

#include "Util/Geometry/geomUtil.h"
#include "Graphics/serial/Scene/GraphicsEntity.h"
#include "Graphics/serial/Scene/GraphicsScene.h"

namespace illGraphics {

void GraphicsEntity::GraphicsNode::rootMove(const glm::vec3& position) {
    assert(m_state == State::IN_SCENE);
    assert(getParent() == NULL);
    
    Box<> prevBounds(getFullBoundingVol());

    m_transform = setTransformPosition(m_transform, position);
    computeFullTransforms();

    m_scene->moveNode(this, prevBounds);
}

void GraphicsEntity::GraphicsNode::rootMove(const glm::mat4& transform) {
    assert(m_state == State::IN_SCENE);
    assert(getParent() == NULL);

    Box<> prevBounds(getFullBoundingVol());

    m_transform = transform;
    computeFullTransforms();

    m_scene->moveNode(this, prevBounds);
}

void GraphicsEntity::GraphicsNode::rootMove(const Box<>& boundingBox) {
    assert(m_state == State::IN_SCENE);
    assert(getParent() == NULL);

    Box<> prevBounds(getFullBoundingVol());

    m_boundingVol = boundingBox;
    computeFullTransforms();

    m_scene->moveNode(this, prevBounds);
}

void GraphicsEntity::GraphicsNode::rootMove(const glm::vec3& position, const Box<>& boundingBox) {
    assert(m_state == State::IN_SCENE);
    assert(getParent() == NULL);

    Box<> prevBounds(getFullBoundingVol());

    m_transform = setTransformPosition(m_transform, position);
    m_boundingVol = boundingBox;
    computeFullTransforms();

    m_scene->moveNode(this, prevBounds);
}

void GraphicsEntity::GraphicsNode::rootMove(const glm::mat4& transform, const Box<>& boundingBox) {
    assert(m_state == State::IN_SCENE);
    assert(getParent() == NULL);

    Box<> prevBounds(getFullBoundingVol());

    m_transform = transform;
    m_boundingVol = boundingBox;
    computeFullTransforms();

    m_scene->moveNode(this, prevBounds);
}

void GraphicsEntity::GraphicsNode::rootAddToScene() {
    assert(m_state == State::OUT_SCENE);
    assert(getParent() == NULL);

    computeFullTransforms();

    m_scene->addNode(this);
    m_state = State::IN_SCENE;
}

void GraphicsEntity::GraphicsNode::rootRemoveFromScene() {
    assert(m_state == State::IN_SCENE);
    assert(getParent() == NULL);

    computeFullTransforms();

    m_scene->removeNode(this);
    m_state = State::OUT_SCENE;
}

void GraphicsEntity::GraphicsNode::childMove(const glm::mat4& relativeTransform) {
    assert(m_state == State::OUT_SCENE);
    assert(getParent() != NULL);

    m_transform = relativeTransform;
}

void GraphicsEntity::GraphicsNode::childMove(const Box<>& boundingBox) {
    assert(m_state == State::OUT_SCENE);
    assert(getParent() != NULL);

    m_boundingVol = boundingBox;
}

void GraphicsEntity::GraphicsNode::childMove(const glm::mat4& relativeTransform, const Box<>& boundingBox) {
    assert(m_state == State::OUT_SCENE);
    assert(getParent() != NULL);

    m_transform = relativeTransform;
    m_boundingVol = boundingBox;
}

}