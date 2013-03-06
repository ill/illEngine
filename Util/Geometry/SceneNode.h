#ifndef ILL_SCENE_NODE_H_
#define ILL_SCENE_NODE_H_

#include <cassert>
#include <set>
#include "Util/Geometry/geomUtil.h"
#include "Util/Geometry/Box.h"

template <typename T = glm::mediump_float>
class SceneNode {
public:
    typedef std::set<SceneNode *> ChildSet;

    /**
    Gets the node's world transform.
    Be sure to call computeFullTransforms on the root node in the heirarchy when ready to properly set this.
    */
    inline const glm::detail::tmat4x4<T>& getFullTransform() const {
        return m_fullTransform;
    }

    /**
    Gets the node's bounding box in world space.
    Be sure to call computeFullTransforms on the root node in the heirarchy when ready to properly set this.
    This is what's used to correctly position the node in the scene spatial datastructure.
    */
    inline const Box<T>& getFullBoundingVol() const {
        return m_fullBoundingVol;
    }
    
    /**
    Gets the constant parent of this node.
    NULL if no parent.
    */
    inline SceneNode * getParent() const {
        return m_parent;
    }

    /**
    Gets the children of this node.
    */
    inline const ChildSet& getChildren() const {
        return m_children;
    }

    /**
    Gets the root node in the heirarchy.
    */
    inline SceneNode * getRootNode() {
        SceneNode * res = this;

        while(res->m_parent) {
            res = res->m_parent;
        }

        return res;
    }

protected:
    SceneNode(const glm::detail::tmat4x4<T>& transform, const Box<T>& boundingVol)
        : m_parent(NULL),
        m_transform(transform),
        m_boundingVol(boundingVol)
    {}

    /**
    Computes the full world space transforms and bounding boxes of the node heirarchy.
    Call this on the root node only, the one that has no parents, or errors will happen.
    */
    inline void computeFullTransforms() {
        assert(!m_parent);
        m_fullTransform = m_transform;

        computeFullTransformsRecursive();
    }

    /**
    Adds a child to this node.
    Be sure the node being added isn't a child of another node already or else errors will happen.
    */
    void addChild(SceneNode * node) {
        assert(!node->m_parent);
        assert(m_children.find(node) == m_children.end());

        node->m_parent = this;
        m_children.insert(node);
    }

    /**
    Removes a child from this node.
    Errors will happen if the node isn't already a child.
    */
    void removeChild(SceneNode * node) {
        assert(node->m_parent == this);
        assert(m_children.find(node) != m_children.end());

        node->m_parent = NULL;
        m_children.erase(node);
    }

    /**
    Gets called when the full bounding volume changes when doing computeFullTransformsRecursive()
    */
    virtual void onFullBoundingVolChange(const Box<T> lastFullBoundingVol) {}

    /**
    The node's relative transform to its parent, or to the world if no parent.
    */
    glm::detail::tmat4x4<T> m_transform;

    /**
    The node's local bounding box not relative to the world.
    To find it relative to the world, offset it by the transform.
    */
    Box<T> m_boundingVol;

private:
    void computeFullTransformsRecursive() {
        //set up the full bounding volume
        Box<T> lastFullBoundingVol(m_fullBoundingVol);
        m_fullBoundingVol = m_boundingVol + getTransformPosition(m_fullTransform);
        onFullBoundingVolChange(lastFullBoundingVol);

        //compute all transforms of the children
        for(auto iter = m_children.begin(); iter != m_children.end(); iter++) {
            SceneNode * node = *iter;

            node->m_fullTransform = m_fullTransform * node->m_transform;
            node->computeFullTransformsRecursive();
        }
    }

    SceneNode * m_parent;
    ChildSet m_children;
        
    glm::detail::tmat4x4<T> m_fullTransform;
    Box<T> m_fullBoundingVol;
};

#endif