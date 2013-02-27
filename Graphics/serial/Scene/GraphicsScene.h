#ifndef ILL_GRAPHICS_SCENE_H_
#define ILL_GRAPHICS_SCENE_H_

#include <unordered_set>

#include "Util/Geometry/GridVolume3D.h"
#include "Util/Geometry/SceneNode.h"
#include "Util/serial/Array.h"
#include "Util/Geometry/BoxIterator.h"
#include "Util/Geometry/BoxOmitIterator.h"
#include "Graphics/serial/Scene/GraphicsEntity.h"

#include "Logging/logging.h"

template <typename T = glm::mediump_float>
struct Frustum;

namespace illGraphics {

class RenderArgs;

class GraphicsScene {
public:
    /**
    Creates the scene and its 3D uniform grid.

    @param cellDimensions The dimensions of the grid cells in world units.
    @param cellNumber The number of cells in each dimension.
    */
    inline GraphicsScene(const glm::vec3& cellDimensions, const glm::uvec3& cellNumber)
        : m_accessCounter(0),
        m_grid(cellDimensions, cellNumber)
    {
        m_nodes = new NodeContainer[cellNumber.x * cellNumber.y * cellNumber.z];
        m_staticNodes = new StaticNodeContainer[cellNumber.x * cellNumber.y * cellNumber.z];
        m_lights = new NodeContainer[cellNumber.x * cellNumber.y * cellNumber.z];
        m_staticLights = new StaticNodeContainer[cellNumber.x * cellNumber.y * cellNumber.z];
    }

    inline ~GraphicsScene() {
        delete[] m_nodes;
        delete[] m_staticNodes;
        delete[] m_lights;
        delete[] m_staticLights;
    }

    /**
    As you are about to render a viewport, call this to retreive what object are visible in that viewport.
    This does view frustum and occlusion culling on the scene from a point of view.

    You can even reuse the objects returned in this call in a totally different view port to debug draw the effectiveness of the occlusion culling
    and see only what's visible from another camera angle.

    @param viewFrustum The view frustum computed from a camera when you set the camera's transform.
    @param renderArgsDest This is the structure you will pass in to the Renderer::render method.
    It contains the nodes in the scene and will be rendered.
    */
    void viewCull(const Frustum<>& viewFrustum, RenderArgs& renderArgsDest);

    /**
    Returns the grid volume that is used to manage the scene.
    */
    const GridVolume3D<>& getGridVolume() const {
        return m_grid;
    }

private:
    typedef std::unordered_set<GraphicsEntity::GraphicsNode*> NodeContainer;
    typedef Array<GraphicsEntity::GraphicsNode*> StaticNodeContainer;

    inline void addNode(GraphicsEntity::GraphicsNode * node) {
        BoxIterator<> iter = m_grid.boxIterForWorldBounds(node->getFullBoundingVol());

        NodeContainer * nodeContainer = nodeCollectionForType(node->getType());
    
        while(!iter.atEnd()) {
            nodeContainer[m_grid.indexForCell(iter.getCurrentPosition())].insert(node);
            iter.forward();
        }
    }

    inline void removeNode(GraphicsEntity::GraphicsNode * node) {
        BoxIterator<> iter = m_grid.boxIterForWorldBounds(node->getFullBoundingVol());

        NodeContainer * nodeContainer = nodeCollectionForType(node->getType());

        while(!iter.atEnd()) {
            nodeContainer[m_grid.indexForCell(iter.getCurrentPosition())].erase(node);
            iter.forward();
        }
    }

    inline void moveNode(GraphicsEntity::GraphicsNode * node, const Box<>& prevBounds) {
        //remove
        {
            BoxOmitIterator<> iter = m_grid.boxOmitIterForWorldBounds(prevBounds, node->getFullBoundingVol());

            NodeContainer * nodeContainer = nodeCollectionForType(node->getType());
    
            while(!iter.atEnd()) {
                nodeContainer[m_grid.indexForCell(iter.getCurrentPosition())].insert(node);
                iter.forward();
            }
        }

        //add
        {
            BoxOmitIterator<> iter = m_grid.boxOmitIterForWorldBounds(node->getFullBoundingVol(), prevBounds);

            NodeContainer * nodeContainer = nodeCollectionForType(node->getType());

            while(!iter.atEnd()) {
                nodeContainer[m_grid.indexForCell(iter.getCurrentPosition())].erase(node);
                iter.forward();
            }
        }
    }

    inline NodeContainer * nodeCollectionForType(GraphicsEntity::GraphicsNode::Type type) {
        switch(type) {
        case GraphicsEntity::GraphicsNode::Type::GENERIC:
            return m_nodes;

        case GraphicsEntity::GraphicsNode::Type::LIGHT:
            return m_lights;

        default:
            LOG_FATAL_ERROR("Invalid Node Type");
            return NULL;
        }
    }

    inline StaticNodeContainer * staticNodeCollectionForType(GraphicsEntity::GraphicsNode::Type type) {
        switch(type) {
        case GraphicsEntity::GraphicsNode::Type::GENERIC:
            return m_staticNodes;

        case GraphicsEntity::GraphicsNode::Type::LIGHT:
            return m_staticLights;

        default:
            LOG_FATAL_ERROR("Invalid Node Type");
            return NULL;
        }
    }

    /**
    The counter that keeps track of which nodes have been accessed in some way during some call.
    This is to handle nodes that overlap multiple cells in the grid and keeps them from being processed more than once.
    */
    uint64_t m_accessCounter;

    /**
    The 3D uniform grid for the scene.
    */
    GridVolume3D<> m_grid;

    /**
    The list of moving nodes per grid cell.
    */
    NodeContainer * m_nodes;

    /**
    The list of static nodes per grid cell.
    */
    StaticNodeContainer * m_staticNodes;

    /**
    The list of moving lights per grid cell.
    */
    NodeContainer * m_lights;

    /**
    The list of static lights per grid cell.
    */
    StaticNodeContainer * m_staticLights;

    friend class GraphicsEntity::GraphicsNode;
};

}

#endif