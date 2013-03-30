#ifndef ILL_BOX_OMIT_ITERATOR_H_
#define ILL_BOX_OMIT_ITERATOR_H_

#include "Box.h"

/**
Iterates over some box range while also omitting a box range.
@tparam P The precision of the 3d grid volume cell subdivision
*/
template <typename P = unsigned int>
class BoxOmitIterator {
public:
    BoxOmitIterator() {}

    BoxOmitIterator(const Box<P>& range, const Box<P>& omitRange)
        : m_range(range),
        m_omitRange(omitRange),
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
        
        //TODO: optimize this

        while(true) {
            //advance
            for(unsigned int dimension = 0; dimension < 3; dimension++) {
                if(m_currPos[dimension] == m_range.m_max[dimension]) {
                    m_currPos[dimension] = m_range.m_min[dimension];
                }
                else {
                    m_currPos[dimension]++;
                    break;
                }
            }

            //check if within the omit range
            if(m_omitRange.intersects(m_currPos)) {
                if(m_omitRange.m_max.z < m_range.m_max.z) {
                    m_currPos.z = m_omitRange.m_max.z + 1;
                    break;
                }
                else {
                    m_currPos.z = m_range.m_max.z;
                }
            }
            else {
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
    Gets the iterator range that will be omitted.
    */
    inline const Box<P>& getOmitRange() const {
        return m_omitRange;
    }

    /**
    Gets the current position in the iterator.
    */
    inline const glm::detail::tvec3<P>& getCurrentPosition() const {
        return m_currPos;
    }

private:            
    Box<P> m_range;
    Box<P> m_omitRange;
    glm::detail::tvec3<P> m_currPos;
};

#endif