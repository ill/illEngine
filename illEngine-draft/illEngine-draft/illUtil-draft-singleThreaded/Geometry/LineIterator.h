#ifndef __LINE_ITERATOR_H__
#define __LINE_ITERATOR_H__

#include <glm/glm.hpp>
#include "geomUtil.h"

/**
Uses DDA line rasterizing algorithm to iterate from one point to another in a line.

@tparam T The precision for space being traversed in a line.
*/
template <typename T = int>
class LineIterator {
public:
    //TODO: make debugger stuff debug build only
    struct Debugger {
        struct Traversal {
            Traversal() {}

            Traversal(const glm::detail::tvec3<T>& start, const glm::detail::tvec3<T>& end)
                : m_start(start),
                m_end(end)
            {}

            glm::detail::tvec3<T> m_start;
            glm::detail::tvec3<T> m_end;
        };

        std::list<Traversal> m_traversals;
    };

    LineIterator() {}

    /**
    Creates the iterator
    @param start The starting cell in the grid.
    This can be outside of the grid volume.  The line algorithm will clip to the insides of the volume.
    @param end The ending cell in the grid
    This can be outside of the grid volume.  The line algorithm will clip to the insides of the volume.
    */
    LineIterator(const glm::detail::tvec3<T>& start, const glm::detail::tvec3<T>& end)
        : m_start(start),
        m_end(end),
        m_current(start)
    {
        m_currentStep.x = (glm::mediump_float) m_current.x;
        m_currentStep.y = (glm::mediump_float) m_current.y;
        m_currentStep.z = (glm::mediump_float) m_current.z;

        m_step.x = (glm::mediump_float) end.x - start.x;
        m_step.y = (glm::mediump_float) end.y - start.y;
        m_step.z = (glm::mediump_float) end.z - start.z;

        glm::mediump_float multiplier = glm::max(glm::abs(m_step.x), glm::abs(m_step.y));
        multiplier = glm::max(multiplier, glm::abs(m_step.z));

        m_step /= multiplier;
    }

    bool operator ==(const LineIterator& other) const {
        return m_start == other.m_start
            && m_end == other.m_end
            && m_current == other.m_current
            && m_currentStep == other.m_currentStep;
    }

    bool operator !=(const LineIterator& other) const {
        return !(*this == other);
    }

    /**
    Checks if currently at the start of the line
    */
    inline bool atStart() const {
        return m_current == m_start;
    }

    /**
    Checks if currently at the end of the line
    */
    inline bool atEnd() const {
        return m_current == m_end;
    }

    /**
    Moves forward along the line

    @return Whether or not moved forward or at the end of the line.
    */
    inline bool forward() {
        if(atEnd()) {
            return false;
        }

        m_currentStep += m_step;

        glm::detail::tvec3<T> current = m_current;

        round();

        m_debugger.m_traversals.push_back(Debugger::Traversal(current, m_current));

        return true;
    }

    /**
    Moves backward along the line

    @return Whether or not moved backward or at the start of the line.
    */
    inline bool back() {
        if(atStart()) {
            return false;
        }

        m_currentStep -= m_step;

        round();

        return true;
    }

    const glm::detail::tvec3<T>& getCurrentPosition() const {
        return m_current;
    }

    const glm::detail::tvec3<T>& getStart() const {
        return m_start;
    }

    const glm::detail::tvec3<T>& getEnd() const {
        return m_end;
    }

    Debugger m_debugger;

private:
    void round() {
        m_current = vec3cast<glm::mediump_float, T>(glm::round(m_currentStep));
    }

    glm::detail::tvec3<T> m_start;
    glm::detail::tvec3<T> m_end;
    glm::detail::tvec3<T> m_current;

    glm::vec3 m_currentStep;
    glm::vec3 m_step;
};

#endif