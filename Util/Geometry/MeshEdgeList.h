#ifndef __MESH_EDGE_LIST_H__
#define __MESH_EDGE_LIST_H__

#include <glm/glm.hpp>
#include <vector>

/**
Used for storing the points and edges joining the points in a 3D mesh.
It's currently limited to 256 points because this is used for some 
internal volume representations and 3D rasterizations which aren't likely to need more points than that.
*/
template<typename T = glm::mediump_float>
struct MeshEdgeList {
    typedef uint8_t PointIndex;

    struct Edge {
        Edge(PointIndex ptA, PointIndex ptB)
            : m_ptA(ptA),
            m_ptB(ptB)
        {}

        PointIndex m_ptA;
        PointIndex m_ptB;
    };

    inline Edge& edgeForPoint(PointIndex pointIndex)

        std::list<glm::detail::tvec3<T> > m_points;
    std::list<Edge> m_edges;
};

#endif