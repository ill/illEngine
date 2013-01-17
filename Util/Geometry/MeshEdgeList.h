#ifndef ILL_MESH_EDGE_LIST_H__
#define ILL_MESH_EDGE_LIST_H__

#include <glm/glm.hpp>
#include <unordered_map>
#include <unordered_set>

#include "Util/serial/VectorManager.h"
#include "Util/Geometry/Plane.h"

/**
Used for storing the points and edges joining the points in a 3D mesh.
*/
template<typename T = glm::mediump_float>
struct MeshEdgeList {
    typedef std::unordered_multimap<size_t, size_t> PointEdgeMap;
    typedef PointEdgeMap::const_iterator PointEdgeMapIterator;
    typedef std::pair<PointEdgeMapIterator, PointEdgeMapIterator> PointEdgeMapIterators;

    struct Edge {
        Edge() {}

        Edge(size_t ptA, size_t ptB)
            : m_ptA(ptA),
            m_ptB(ptB)
        {}

        size_t m_ptA;
        size_t m_ptB;
    };

    /**
    Computes the point edge map for fast edge lookup by point.
    */
    inline void computePointEdgeMap() {
        m_pointEdgeMap.clear();
        m_pointEdgeMap.rehash(m_points.size());

        for(size_t edgeIndex = 0; edgeIndex < m_edges.size(); edgeIndex++) {
            m_pointEdgeMap.insert(m_edges[edgeIndex].m_ptA, edgeIndex);
            m_pointEdgeMap.insert(m_edges[edgeIndex].m_ptB, edgeIndex);
        }
    }

    /**
    Clips the mesh edge list against a plane.
    */
    void clip(const Plane<T>& clipPlane) const {
        //find which side of the plane points are on
        std::unordered_set<size_t> offsidePoints(m_points.size());

        for(size_t pointIndex = 0; pointIndex < m_points.size(); pointIndex++) {
            if(!clipPlane.pointOnSide(m_points[pointIndex])) {
                offsidePoints.insert(pointIndex);
            }
        }

        if(offsidePoints.empty()) {        //if all points are onside, no clipping needed
            return;
        }
        else if(offsidePoints.size() == m_points.size()) {                 //if all points are offside, the entire polygon is gone
            m_points.clear();
            m_edges.clear();
            m_pointEdgeMap.clear();

            return;
        }

        //now either clip lines that intersect the plane, or completely remove them
        struct ClippedEdge {            
            size_t m_ptA;
            glm::detail::tvec3 m_ptB;
        };

        std::unordered_set<size_t> modifiedEdges(m_edges.size());                       //edges that were modified by being clipped or removed
        std::unordered_map<size_t, ClippedEdge> clippedEdges(m_edges.size());           //edges that were clipped

        for(std::unordered_set<size_t>::const_iterator offsidePtIter = offsidePoints.begin(); offsidePtIter != offsidePoints.end(); offsidePtIter++) {
            size_t offsidePoint = *offsidePtIter;
            
            //get edges for offside point
            PointEdgeMapIterators edges = m_pointEdgeMap.equal_range(offsidePoint);

            for(PointEdgeMapIterator edgeIter = edges.first; edgeIter != edges.second; edgeIter++) {
                size_t edgeIndex = edgeIter->second;
                
                //if edge already modified, skip it
                if(modifiedEdges.find(edgeIndex) == modifiedEdges.end()) {
                    continue;
                }

                modifiedEdges.insert(edgeIndex);

                Edge& edge = m_edges[edgeIndex];
                
                //find the other point
                size_t otherPoint = offsidePoint == edge.m_ptA 
                    ? otherPoint = edge.m_ptB
                    : otherPoint = edge.m_ptA;

                //find if other point is onside or offside
                std::unordered_set<size_t>::const_iterator otherOffsideIter = offsidePoints.find(otherPoint);
                                
                if(otherOffsideIter == offsidePoints.end()) {   //other point is onside, so clip this edge
                    ClippedEdge clippedEdge;
                    clippedEdge.m_ptA = otherPoint;
                    
                    bool intersection = lineIntersection(m_points.get(offsidePoint), m_points.get(otherPoint), clippedEdge.m_ptB);
                    assert(intersection);   //if there's no intersection, something went seriously wrong

                    clippedEdges.insert(edgeIndex, clippedEdge);
                }
                else {
                    //if other point is offside, the edge is removed
                    m_edges.remove(edgeIndex);
                }
            }
        }

        //go through clipped edges and replace old edges with clipped edges, while also setting up the convex hull algorithm to create new joining edges

    }

    VectorManager<glm::detail::tvec3<T> > m_points;
    VectorManager<Edge> m_edges;

    /**
    Fast point to edge lookup map.
    */
    PointEdgeMap m_pointEdgeMap;
};

#endif