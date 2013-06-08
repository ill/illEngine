#ifndef ILL_MULTI_CONVEX_MESH_ITERATOR_H_
#define ILL_MULTI_CONVEX_MESH_ITERATOR_H_

#include <vector>
#include "Util/Geometry/Iterators/ConvexMeshIterator.h"
#include "Logging/logging.h"

/**
A wrapper around the ConvexMeshIterator but holds multiple convex mesh iterators.
This is good for when the ConvexMesh had to be split up somehow, or maybe a volume
is made of multiple Convex Meshes.

Just add mesh iterators to the arrays, ideally by just preallocating the array.
Then call advance to advance through all iterators.  If any iterators in here overlap or have diagonal
boundaries, there is nothing stopping those cells to be iterated over multiple times.

You can also hold the mesh edge list copies here if they were needed.

This is a VERY simple class without too much encapsulation or safety, so just use it right.
*/
template <typename W = glm::mediump_float, typename P = unsigned int>
class MultiConvexMeshIterator {
public:
    MultiConvexMeshIterator()
        : m_currentIter(0)
    {}
    
    MultiConvexMeshIterator(const MultiConvexMeshIterator& other)
        : m_currentIter(other.m_currentIter),
        m_meshEdgeListCopies(other.m_meshEdgeListCopies),
        m_iterators(other.m_iterators)
    {
        //make the iterators point at the local mesh edge list copies
        for(size_t iter = 0; iter < m_iterators.size(); iter++) {
            m_iterators[iter].m_meshEdgeList = &m_meshEdgeListCopies[iter];
        }
    }

    /**
    The current grid cell the iterator is on.
    */
    inline glm::detail::tvec3<P> getCurrentPosition() const {
        if(atEnd()) {
            LOG_FATAL_ERROR("calling getCurrentPosition() on multimesh iterator when at end");
        }

        return m_iterators[m_currentIter].getCurrentPosition();
    }

    /**
    Whether or not all cells in the iterator have been rasterized.
    */
    inline bool atEnd() const {
        return m_currentIter >= m_iterators.size() 
            || (m_currentIter == m_iterators.size() - 1 && m_iterators[m_currentIter].atEnd());
    }

    /**
    Moves the iterator forward.
    */
    inline void forward() {
        if(atEnd()) {
            LOG_FATAL_ERROR("calling forward() on multimesh iterator when at end");
        }

        if(m_iterators[m_currentIter].atEnd()) {
            m_currentIter++;
        }
        else {
            m_iterators[m_currentIter].forward();

            if(m_iterators[m_currentIter].atEnd()) {
                m_currentIter++;
            }
        }
    }

    size_t m_currentIter;
    std::vector<MeshEdgeList<W>> m_meshEdgeListCopies;
    std::vector<ConvexMeshIterator<W, P>> m_iterators; 
};

#endif