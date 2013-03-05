#ifndef ILL_GRAPHICS_SCENE_H_
#define ILL_GRAPHICS_SCENE_H_

#include <unordered_set>

#include "Util/Geometry/GridVolume3D.h"
#include "Util/Geometry/SceneNode.h"
#include "Util/serial/Array.h"
#include "Util/Geometry/BoxIterator.h"
#include "Util/Geometry/BoxOmitIterator.h"
#include "RendererCommon/serial/GraphicsEntity.h"
#include "Graphics/serial/Light.h"

#include "Logging/logging.h"

namespace illGraphics {
class Camera;
class Mesh;
class Material;
}

namespace illRendererCommon {

class GraphicsScene {
public:
    virtual inline ~GraphicsScene() {
        delete[] m_nodes;
        delete[] m_staticNodes;
        delete[] m_lights;
        delete[] m_staticLights;
    }

    /**
    Render a scene from a camera angle.  This interacts with the renderer backend directly.
    */
    virtual void render(const illGraphics::Camera& camera) = 0;

    /**
    Returns the grid volume that is used to manage the scene.
    */
    const GridVolume3D<>& getGridVolume() const {
        return m_grid;
    }

protected:
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

    typedef std::unordered_set<GraphicsEntity::GraphicsNode*> NodeContainer;
    typedef Array<GraphicsEntity::GraphicsNode*> StaticNodeContainer;

    struct RenderArgs {       
        struct MeshInfo {
            std::vector<illGraphics::LightBase *> m_affectingLights;
            glm::mat4 m_transform;
            illGraphics::Mesh * m_mesh;
        };

        struct LightInfo {
            glm::mat4 m_transform;
            illGraphics::LightBase * m_light;
        };
    
        std::unordered_map<illGraphics::Material *, MeshInfo> m_solidMeshes;
        std::unordered_map<illGraphics::Material *, MeshInfo> m_unsolidMeshes;
        std::unordered_map<illGraphics::Material *, MeshInfo> m_unsolidDepthsortedMeshes;

        std::unordered_map<illGraphics::LightBase::Type, LightInfo> m_lights;
    };

private:
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

protected:
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

    //TODO: add a finer grid for tracking lights that's used specifically for finding lights close to an object for forward rendering

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