#include <cassert>

#include "RendererCommon/serial/GraphicsNode.h"
#include "RendererCommon/serial/GraphicsScene.h"

namespace illRendererCommon {

void GraphicsNode::move(const glm::vec3& position) {
    assert(m_state == State::IN_SCENE);
    
    Box<> prevBounds(getWorldBoundingVolume());
    m_transform = setTransformPosition(m_transform, position);
    m_scene->moveNode(this, prevBounds);
}

void GraphicsNode::move(const glm::mat4& transform) {
    assert(m_state == State::IN_SCENE);

    Box<> prevBounds(getWorldBoundingVolume());
    m_transform = transform;
    m_scene->moveNode(this, prevBounds);
}

void GraphicsNode::move(const Box<>& boundingVolume) {
    assert(m_state == State::IN_SCENE);

    Box<> prevBounds(getWorldBoundingVolume());
    m_boundingVol = boundingVolume;
    m_scene->moveNode(this, prevBounds);
}

void GraphicsNode::move(const glm::vec3& position, const Box<>& boundingVolume) {
    assert(m_state == State::IN_SCENE);

    Box<> prevBounds(getWorldBoundingVolume());
    m_transform = setTransformPosition(m_transform, position);
    m_boundingVol = boundingVolume;
    m_scene->moveNode(this, prevBounds);
}

void GraphicsNode::move(const glm::mat4& transform, const Box<>& boundingVolume) {
    assert(m_state == State::IN_SCENE);

    Box<> prevBounds(getWorldBoundingVolume());
    m_transform = transform;
    m_boundingVol = boundingVolume;
    m_scene->moveNode(this, prevBounds);
}

void GraphicsNode::addToScene() {
    assert(m_state == State::OUT_SCENE);
    m_scene->addNode(this);
    m_state = State::IN_SCENE;
}

void GraphicsNode::removeFromScene() {
    assert(m_state == State::IN_SCENE);
    m_scene->removeNode(this);
    m_state = State::OUT_SCENE;
}

}