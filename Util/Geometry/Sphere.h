#ifndef ILL_SPHERE_H__
#define ILL_SPHERE_H__

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

    /**
     * Checks for a ray intersection with the sphere.
     *
     * @param rayOrigin The origin of the ray
     * @param rayDirection The direction of the ray
     * @param distance If a collision happened, this is the closest distance down the ray where the intersection occured.
     *
     * @return Whether or not an intersection happened.  Check this before using the intersection distance.
     *
     * Code modified from here http://wiki.cgsociety.org/index.php/Ray_Sphere_Intersection
     */
    inline bool rayIntersection(glm::detail::tvec3<T> rayOrigin, const glm::detail::tvec3<T>& rayDirection, T& distance) const {
        //put the ray into object space of the sphere (TODO: make sure this is correct)
        rayOrigin -= m_center;

        //Compute A, B and C coefficients
        T a = glm::dot(rayDirection, rayDirection);
        T b = (T)2 * glm::dot(rayDirection, rayOrigin);
        T c = glm::dot(rayOrigin, rayOrigin) - (m_radius * m_radius);

        //Find discriminant
        T disc = b * b - (T)4 * a * c;

        // if discriminant is negative there are no real roots, so return
        // false as ray misses sphere
        if (disc < 0) {
            return false;
        }

        // compute q as described above
        T distSqrt = glm::sqrt(disc);
        T q;
        if (b < 0) {
            q = (-b - distSqrt) / (T)2;
        }
        else {
            q = (-b + distSqrt) / (T)2;
        }

        // compute t0 and t1
        T t0 = q / a;
        T t1 = c / q;

        // make sure t0 is smaller than t1
        if(t0 > t1) {
            // if t0 is bigger than t1 swap them around
            T temp = t0;
            t0 = t1;
            t1 = temp;
        }

        // if t1 is less than zero, the object is in the ray's negative direction
        // and consequently the ray misses the sphere
        if (t1 < (T) 0) {
            return false;
        }

        // if t0 is less than zero, the intersection point is at t1
        if (t0 < (T) 0) {
            distance = t1;
            return true;
        }
        // else the intersection point is at t0
        else
        {
            distance = t0;
            return true;
        }
    }

    ///The center origin point of the sphere
    glm::detail::tvec3<T> m_center;

    ///The radius of the sphere
    T m_radius;
};

#endif
