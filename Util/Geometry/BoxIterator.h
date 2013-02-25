#ifndef ILL_BOX_ITERATOR_H__
#define ILL_BOX_ITERATOR_H__

#include "Box.h"

/**
Iterates over some box range.
@tparam P The precision of the 3d grid volume cell subdivision
*/
template <typename P = unsigned int>
class BoxIterator {
public:
    BoxIterator() {}

    BoxIterator(const Box<P>& range)
        : m_range(range),
        m_currPos(m_range.m_min)
    {}
    
    /**
    Checks if currently at the end of the iterator range.
    */
    inline bool atEnd() const {
        return m_currPos.x >= m_range.m_max.x 
            && m_currPos.y >= m_range.m_max.y
            && m_currPos.z >= m_range.m_max.z;
    }

    /**
    Moves forward.

    @return Whether or not moved forward or at the end of the iterator range.
    */
    inline bool forward() {
        if(atEnd()) {
            return false;
        }

        for(unsigned int dimension = 0; dimension < 3; dimension++) {
            if(m_currPos[dimension] == m_range.m_max[dimension]) {
                m_currPos[dimension] = m_range.m_min[dimension];
            }
            else {
                m_currPos[dimension]++;
                break;
            }
        }

        return true;
    }

    /**
    Checks if currently at the start of the iterator range.
    */
    inline bool atStart() const {
        return m_currCell.x <= m_range.m_min.x 
            && m_currCell.y <= m_range.m_min.y
            & m_currCell.z <= m_range.m_min.z;
    }

    /**
    Moves back.

    @return Whether or not moved back or at the start of the iterator range.
    */
    inline bool back() {
        if(atStart()) {
            return false;
        }

        for(unsigned int dimension = 0; dimension < 3; dimension++) {
            if(m_currPos[dimension] == m_range.m_min[dimension]) {
                m_currPos[dimension] = m_range.m_max[dimension];
            }
            else {
                m_currPos[dimension]--;
                break;
            }
        }

        return true;
    }

    /**
    Gets the iterator range.
    */
    inline const Box<P>& getRange() const {
        return m_range;
    }

    /**
    Gets the current position in the iterator.
    */
    inline const glm::detail::tvec3<P>& getCurrentPosition() const {
        return m_currPos;
    }

private:            
    Box<P> m_range;
    glm::detail::tvec3<P> m_currPos;
};

#endif