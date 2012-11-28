#ifndef __SPHERE_H__
#define __SPHERE_H__

#include <glm/glm.hpp>

/**
Represents a sphere or 3D circular range.
*/
template <typename T = glm::mediump_float>
struct Sphere {
    inline Sphere() {}

    /**
    Copy constructor
    */
    inline Sphere(const Sphere& other)
        : m_center(other.m_center),
        m_radius(other.m_radius)
    {}

    inline Sphere(const glm::detail::tvec3<T>& center, T radius)
        : m_center(center),
        m_radius(radius)
    {}

    ///The center origin point of the sphere
    glm::detail::tvec3<T> m_center;

    ///The radius of the sphere
    T m_radius;
};

#endif