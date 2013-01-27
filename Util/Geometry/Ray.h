#ifndef ILL_RAY_H_
#define ILL_RAY_H_

#include <cassert>
#include <glm/glm.hpp>

template<typename T = glm::mediump_float>
struct Ray {
    Ray() {}

    Ray(const glm::detail::tvec3<T>& origin, const glm::detail::tvec3<T>& direction)
        : m_origin(origin),
        m_direction(direction)
    {
        //assert the direction is normalized
        assert(m_direction == glm::normalize(m_direction));
    }

    glm::detail::tvec3<T> m_origin;
    glm::detail::tvec3<T> m_direction;
};

#endif