#ifndef ILL_MESH_EDGE_LIST_H__
#define ILL_MESH_EDGE_LIST_H__

#include <algorithm>
#include <glm/glm.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

        Edge(size_t ptA, size_t ptB) {
            m_point[0] = ptA;
            m_point[1] = ptB;
        }

        size_t m_point[2];
    };

    inline void clear() {
        m_points.clear();
        m_edges.clear();
        m_pointEdgeMap.clear();
    }

    /**
    Computes the point edge map for fast edge lookup by point.
    */
    inline void computePointEdgeMap() {
        m_pointEdgeMap.clear();
        m_pointEdgeMap.rehash(m_points.size());

        for(size_t edgeIndex = 0; edgeIndex < m_edges.size(); edgeIndex++) {
            m_pointEdgeMap.insert(std::pair<size_t, size_t>(m_edges[edgeIndex].m_point[0], edgeIndex));
            m_pointEdgeMap.insert(std::pair<size_t, size_t>(m_edges[edgeIndex].m_point[1], edgeIndex));
        }
    }

    /**
    Clips the mesh edge list against a plane.
    This only works if this is a convex mesh.
    */
    void convexClip(const Plane<T>& clipPlane) {
        //find which side of the plane points are on
        std::unordered_set<size_t> offsidePoints(m_points.size());

        bool * isPointOffside = new bool[m_points.size()];                               //whether or not a point for an index is offside

        {
            size_t numOffsidePoints = 0;

            for(size_t pointIndex = 0; pointIndex < m_points.size(); pointIndex++) {
                if(!clipPlane.pointOnSide(m_points[pointIndex])) {
                    isPointOffside[pointIndex] = true;
                    numOffsidePoints++;
                }
                else {
                    isPointOffside[pointIndex] = false;
                }
            }

            if(numOffsidePoints == 0) {        //if all points are onside, no clipping needed
                delete[] isPointOffside;
                return;
            }
            else if(numOffsidePoints == m_points.size()) {                 //if all points are offside, the entire polygon is gone
                clear();

                delete[] isPointOffside;
                return;
            }
        }

        //backup the old edges and points so the original data can be written to
        std::vector<glm::detail::tvec3<T> > points;
        std::vector<Edge> edges;
        
        points.swap(m_points);
        edges.swap(m_edges);

        /*points.assign(m_points.begin(), m_points.end());
        edges.assign(m_edges.begin(), m_edges.end());

        m_points.clear();
        m_edges.clear();*/

        //now either clip lines that intersect the plane, or completely remove them
        struct ClippedEdge {            
            bool m_modifiedPointIndex;      //which point in the line is clipped off, 0 or 1
            glm::detail::tvec3<T> m_coords;    //the new coordinates replacing the clipped off point
        };
                
        bool * isEdgeModified = new bool[edges.size()];                                 //whether or not an edge for an index was modified
        memset(isEdgeModified, 0, edges.size() * sizeof(bool));

        size_t * oldPointRemap = new size_t[points.size()];                             //remapping of old point index to new point index

        std::unordered_map<size_t, ClippedEdge> clippedEdges(edges.size());             //edges that were clipped
        
        for(size_t point = 0; point < points.size(); point++) {
            //if point is offside
            if(isPointOffside[point]) {            
                //get edges for offside point
                PointEdgeMapIterators edgeIters = m_pointEdgeMap.equal_range(point);

                for(PointEdgeMapIterator edgeIter = edgeIters.first; edgeIter != edgeIters.second; edgeIter++) {
                    size_t edgeIndex = edgeIter->second;
                
                    //if edge already modified, skip it
                    if(isEdgeModified[edgeIndex]) {
                        continue;
                    }

                    isEdgeModified[edgeIndex] = true;

                    Edge& edge = edges[edgeIndex];
                
                    //find the other point
                    bool modifiedPointIndex = point == edge.m_point[1];
                    size_t otherPoint = edge.m_point[!modifiedPointIndex];

                    //if other point onside, clip the line, otherwise just discard it
                    if(!isPointOffside[otherPoint]) {
                        ClippedEdge clippedEdge;
                        clippedEdge.m_modifiedPointIndex = modifiedPointIndex;
                    
                        bool intersection = clipPlane.lineIntersection(points[point], points[otherPoint], clippedEdge.m_coords);
                        assert(intersection);   //if there's no intersection, something went seriously wrong

                        clippedEdges[edgeIndex] = clippedEdge;
                    }
                }
            }
            else {      //if point is onside readd it
                oldPointRemap[point] = m_points.size();
                m_points.push_back(points[point]);
            }
        }

        //go through unmodified edges and readd them while also remapping their point indeces
        for(size_t edge = 0; edge < edges.size(); edge++) {
            if(!isEdgeModified[edge]) {
                const Edge& oldEdge = edges[edge];
                m_edges.push_back(Edge(oldPointRemap[oldEdge.m_point[0]], oldPointRemap[oldEdge.m_point[1]]));
            }
        }

        //go through clipped edges and add them, while also setting up the convex hull algorithm to create new joining edges
        std::vector<size_t> newPoints;
        newPoints.reserve(clippedEdges.size());

        for(std::unordered_map<size_t, ClippedEdge>::const_iterator clippedEdgeIter = clippedEdges.begin(); clippedEdgeIter != clippedEdges.end(); clippedEdgeIter++) {
            size_t clippedEdgeIndex = clippedEdgeIter->first;
            const ClippedEdge& clippedEdge = clippedEdgeIter->second;
            
            //add the new point
            size_t newPointIndex = m_points.size();
            newPoints.push_back(newPointIndex);

            m_points.push_back(clippedEdge.m_coords);

            //add the new edge
            m_edges.push_back(Edge());
            Edge& newEdge = m_edges.back();

            //set the new edge's points
            newEdge.m_point[clippedEdge.m_modifiedPointIndex] = newPointIndex;
            newEdge.m_point[!clippedEdge.m_modifiedPointIndex] = oldPointRemap[edges[clippedEdgeIndex].m_point[!clippedEdge.m_modifiedPointIndex]];
        }

        delete[] isEdgeModified;
        delete[] oldPointRemap;
        delete[] isPointOffside;

        //use the dimension order of the normal to determine how to best perform the convex hull algorithm
        glm::detail::tvec3<uint8_t> normalDimensionOrder = sortDimensions(clipPlane.m_normal);

        struct PointComparator {
            inline PointComparator(const glm::detail::tvec3<uint8_t>& normalDimensionOrder, const std::vector<glm::detail::tvec3<T> >& points)
                : m_normalDimensionOrder(normalDimensionOrder),
                m_points(points)
            {}

            inline bool operator() (size_t ptA, size_t ptB) {
                //sort points by reverse order of magnitude of normal vector in plane to ensure best sorting
                for(uint8_t dimension = 2; dimension < 3; dimension--) {
                    if(m_points[ptA][dimension] < m_points[ptB][dimension]) {
                        return true;
                    }
                }

                return false;
            }

            const glm::detail::tvec3<uint8_t>& m_normalDimensionOrder;
            const std::vector<glm::detail::tvec3<T> >& m_points;
        };

        //sort the new points so convex hull monotone chain can run on them in a bit
        std::sort(newPoints.begin(), newPoints.end(), PointComparator(normalDimensionOrder, m_points));

        //now do monotone chain on the points to find the convex polygon forming the clipped portion
        //TODO: this is known to not work on some cases when the plane isn't just an x, y, or z plane
        //At the moment I don't need to handle planes like this so I'm not fixing this yet
        {
            std::vector<size_t> newEdgeList;
            newEdgeList.reserve(newPoints.size());

            //do one side
            convexHull(newPoints.begin(), newPoints.end(), normalDimensionOrder, newEdgeList);

            //create edges from that
            for(unsigned int newPointIndex = 0; newPointIndex < newEdgeList.size() - 1;) {
                m_edges.push_back(Edge(newEdgeList[newPointIndex], newEdgeList[newPointIndex++]));
            }

            //then the other
            newEdgeList.clear();
            convexHull(newPoints.rbegin(), newPoints.rend(), normalDimensionOrder, newEdgeList);

            //create edges from that
            for(unsigned int newPointIndex = 0; newPointIndex < newEdgeList.size() - 1;) {
                m_edges.push_back(Edge(newEdgeList[newPointIndex], newEdgeList[newPointIndex++]));
            }
        }

        computePointEdgeMap();
    }

private:
    inline glm::detail::tvec2<T> get2dPoint(size_t pointIndex, const glm::detail::tvec3<uint8_t>& dimensionOrder) const {
        const glm::detail::tvec3<T>& point = m_points[pointIndex];

        return glm::detail::tvec2<T>(point[dimensionOrder[2]], point[dimensionOrder[1]]);
    }

    template <typename Iter>
    void convexHull(Iter& iter, Iter& end, const glm::detail::tvec3<uint8_t> dimensionOrder, std::vector<size_t>& destination) const {                  
        if(iter != end) {
            //init some things first
            size_t point = *iter;            
            destination.push_back(point);
            iter++;

            if(iter != end) {
                point = *iter;
                destination.push_back(point);
                iter++;

                //now do the actual loop
                for(; iter != end; iter++) {
                    point = *iter;

                    while(true) {
                        if(destination.size() < 2) {
                            break;
                        }

                        glm::detail::tvec2<T> lastPointA = get2dPoint(destination[destination.size() - 2], dimensionOrder);
                        glm::detail::tvec2<T> lastPointB = get2dPoint(destination[destination.size() - 1], dimensionOrder);

                        glm::detail::tvec2<T> point2D = get2dPoint(point, dimensionOrder);
                    
                        if(cross(lastPointA - point2D, lastPointB - point2D) > (T) 0) {
                            break;
                        }

                        destination.pop_back();
                    }

                    destination.push_back(point);
                }
            }
        }
    }

public:
    //TODO: make accessor functions for this
    std::vector<glm::detail::tvec3<T> > m_points;
    std::vector<Edge> m_edges;

    /**
    Fast point to edge lookup map.
    */
    PointEdgeMap m_pointEdgeMap;
};

#endif