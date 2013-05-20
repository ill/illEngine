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
    
    /**
    The current grid cell the iterator is on.
    */
    inline glm::detail::tvec3<P> getCurrentPosition() const {
        if(atEnd()) {
            LOG_FATAL_ERROR("calling getCurrentPosition() on multimesh iterator when at end");
        }

        return m_iterators[m_currentIter % m_iterators.size()].getCurrentPosition();
    }

    /**
    Whether or not all cells in the iterator have been rasterized.
    */
    inline bool atEnd() const {
        /*return m_currentIter >= m_iterators.size() 
            || (m_currentIter == m_iterators.size() - 1 && m_iterators[m_currentIter].atEnd());*/

        if(m_iterators.empty()) {
            return true;
        }

        for(size_t iter = 0; iter < m_iterators.size(); iter++) {
            if(!m_iterators[iter].atEnd()) {
                return false;
            }
        }

        return true;
    }

    /**
    Moves the iterator forward.
    */
    inline void forward() {
        if(atEnd()) {
            LOG_FATAL_ERROR("calling forward() on multimesh iterator when at end");
        }

        do {
            ++m_currentIter;
        } while(m_iterators[m_currentIter % m_iterators.size()].atEnd());

        //don't advance the iterator yet if it's its first time being switched to
        if(m_currentIter >= m_iterators.size()) {
            int numSkipped = 0;

            //are you ready for some spagghetti code?  TODO: figure out how to unspaghettify it
            while(true) {
                if(m_iterators[m_currentIter % m_iterators.size()].atEnd()) {
                    ++m_currentIter;
                    ++numSkipped;

                    if(numSkipped == m_iterators.size()) {
                        return;
                    }

                    continue;
                }

                m_iterators[m_currentIter % m_iterators.size()].forward();
                
                if(m_iterators[m_currentIter % m_iterators.size()].atEnd()) {
                    ++m_currentIter;
                    ++numSkipped;

                    if(numSkipped == m_iterators.size()) {
                        return;
                    }
                }
                else {
                    return;
                }
            }
        }
    }

    size_t m_currentIter;
    std::vector<MeshEdgeList<W>> m_meshEdgeListCopies;
    std::vector<ConvexMeshIterator<W, P>> m_iterators; 
};

#endif