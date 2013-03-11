#include <cassert>
#include "GraphicsScene.h"
#include "Util/Geometry/ConvexMeshIterator.h"
#include "LightNode.h"

namespace illRendererCommon {

void GraphicsScene::getLights(const Box<>& boundingBox, std::set<LightNode*>& destination) const {    
    BoxIterator<> iter = m_interactionGrid.boxIterForWorldBounds(boundingBox);
    
    while(!iter.atEnd()) {
        //static lights
        {
            StaticLightNodeContainer& cell = m_staticLightNodes[m_grid.indexForCell(iter.getCurrentPosition())];

            for(size_t nodeInd = 0; nodeInd < cell.size(); nodeInd++) {
                LightNode * node = cell[nodeInd];
                assert(node->getType() == GraphicsNode::Type::LIGHT);

                if(node->m_accessCounter <= m_accessCounter) {
                    node->m_accessCounter = m_accessCounter + 1;

                    if(boundingBox.intersects(node->getWorldBoundingVolume())) {
                        destination.insert(node);
                    }
                }
            }
        }

        //moveable lights
        {
            LightNodeContainer& cell = m_lightNodes[m_grid.indexForCell(iter.getCurrentPosition())];

            for(auto nodeIter = cell.cbegin(); nodeIter != cell.cbegin(); nodeIter++) {
                LightNode * node = *nodeIter;
                assert(node->getType() == GraphicsNode::Type::LIGHT);

                if(node->m_accessCounter <= m_accessCounter) {
                    node->m_accessCounter = m_accessCounter + 1;

                    if(boundingBox.intersects(node->getWorldBoundingVolume())) {
                        destination.insert(node);
                    }
                }
            }
        }

        iter.forward();
    }

    ++m_accessCounter;
}

void GraphicsScene::addNode(GraphicsNode * node) {
    //regular nodes
    if(node->getType() != GraphicsNode::Type::LIGHT
            || (m_trackLightsInVisibilityGrid && node->getType() == GraphicsNode::Type::LIGHT)) {
        BoxIterator<> iter = m_grid.boxIterForWorldBounds(node->getWorldBoundingVolume());
       
        do {
            m_sceneNodes[m_grid.indexForCell(iter.getCurrentPosition())].insert(node);
        } while(iter.forward());
    }

    //lights
    if(node->getType() == GraphicsNode::Type::LIGHT) {
        BoxIterator<> iter = m_interactionGrid.boxIterForWorldBounds(node->getWorldBoundingVolume());
                
        do {
            m_lightNodes[m_grid.indexForCell(iter.getCurrentPosition())].insert(static_cast<LightNode *>(node));
        } while(iter.forward());
    }
}

void GraphicsScene::removeNode(GraphicsNode * node) {
    //regular nodes
    if(node->getType() != GraphicsNode::Type::LIGHT
            || (m_trackLightsInVisibilityGrid && node->getType() == GraphicsNode::Type::LIGHT)) {
        BoxIterator<> iter = m_grid.boxIterForWorldBounds(node->getWorldBoundingVolume());
            
        do {
            m_sceneNodes[m_grid.indexForCell(iter.getCurrentPosition())].erase(node);
        } while(iter.forward());
    }

    //lights
    if(node->getType() == GraphicsNode::Type::LIGHT) {
        BoxIterator<> iter = m_interactionGrid.boxIterForWorldBounds(node->getWorldBoundingVolume());
                
        do {
            m_lightNodes[m_grid.indexForCell(iter.getCurrentPosition())].erase(static_cast<LightNode *>(node));
        } while(iter.forward());
    }
}

void GraphicsScene::moveNode(GraphicsNode * node, const Box<>& prevBounds) {
    //remove
    {
        //regular nodes
        if(node->getType() != GraphicsNode::Type::LIGHT
                || (m_trackLightsInVisibilityGrid && node->getType() == GraphicsNode::Type::LIGHT)) {
            BoxOmitIterator<> iter = m_grid.boxOmitIterForWorldBounds(prevBounds, node->getWorldBoundingVolume());

            do {
                m_sceneNodes[m_grid.indexForCell(iter.getCurrentPosition())].insert(node);
            } while(iter.forward());
        }

        //lights
        if(node->getType() == GraphicsNode::Type::LIGHT) {
            BoxOmitIterator<> iter = m_interactionGrid.boxOmitIterForWorldBounds(prevBounds, node->getWorldBoundingVolume());

            do {
                m_lightNodes[m_grid.indexForCell(iter.getCurrentPosition())].insert(static_cast<LightNode *>(node));
            } while(iter.forward());
        }
    }
        
    //add
    {
        //regular nodes
        if(node->getType() != GraphicsNode::Type::LIGHT
                || (m_trackLightsInVisibilityGrid && node->getType() == GraphicsNode::Type::LIGHT)) {
            BoxOmitIterator<> iter = m_grid.boxOmitIterForWorldBounds(node->getWorldBoundingVolume(), prevBounds);

            do {
                m_sceneNodes[m_grid.indexForCell(iter.getCurrentPosition())].erase(node);
            } while(iter.forward());
        }

        //lights
        if(node->getType() == GraphicsNode::Type::LIGHT) {
            BoxOmitIterator<> iter = m_interactionGrid.boxOmitIterForWorldBounds(node->getWorldBoundingVolume(), prevBounds);

            do {
                m_lightNodes[m_grid.indexForCell(iter.getCurrentPosition())].erase(static_cast<LightNode *>(node));
            } while(iter.forward());
        }
    }
}

}