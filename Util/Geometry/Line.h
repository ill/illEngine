#ifndef ILL_LINE_H_
#define ILL_LINE_H_

#include <glm/glm.hpp>

template <typename T = glm::mediump_float>
struct Line {
    Line() {}

    Line(const glm::detail::tvec3<T>& ptA, const glm::detail::tvec3<T>& ptB) {
        m_point[0] = ptA;
        m_point[1] = ptB;
    }
    
    glm::detail::tvec3<T> m_point[2];
};

#endif