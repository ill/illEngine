#ifndef ILL_BOX_H__
#define ILL_BOX_H__

#include <glm/glm.hpp>
#include "Sphere.h"

/**
Represents an axis aligned bounding box or some other axis aligned 3D range.
*/
template <typename T = glm::mediump_float>
struct Box {
    inline Box() {}

    /**
    Copy constructor
    */
    inline Box(const Box& other)
        : m_min(other.m_min),
        m_max(other.m_max)
    {}

    /**
    Constructs box from two corners.
    Doesn't check if min and max are really in the right order.
    That's what Box::normalize is for

    @param min The bottom, left, near corner
    @param max The top, right, far corner
    */
    inline Box(const glm::detail::tvec3<T>& min, const glm::detail::tvec3<T>& max)
        : m_min(min),
        m_max(max)
    {}

    /**
    Constructs a box from a point to be the size of a point.
    */
    inline Box(const glm::detail::tvec3<T>& point)
        : m_min(point),
        m_max(point)
    {}

    /**
    Constructs a box that would contain a sphere
    */
    inline Box(const Sphere<T>& sphere) {
        m_min = sphere.m_center - sphere.m_radius;
        m_max = sphere.m_center + sphere.m_radius;
    }

    /**
    Fixes the box so all coordinate components of m_min are less than m_max

    @return a normalized box
    */
    inline Box normalize() const {
        Box res;

        for(unsigned int coord = 0; coord < 3; coord++) {
            if(m_min[coord] > m_max[coord]) {
                res.m_min[coord] = m_max[coord];
                res.m_max[coord] = m_min[coord];
            }
            else {
                res.m_min[coord] = m_min[coord];
                res.m_max[coord] = m_max[coord];
            }
        }

        return res;
    }

    inline glm::detail::tvec3<T> getDimensions() const {
        return m_max - m_min;
    }

    /**
    Returns which grid coordinates the coordinates of the box snap to given dimensions.
    */
    template <typename R>
    inline Box<R> grid(const glm::detail::tvec3<T>& dimensions) const {
        return Box<R>(gridVec<T, R>(m_min, dimensions), gridVec<T, R>(m_max, dimensions));
    }

    /**
    Adds a point and expands the box.
    Be sure the box is the size of a point when adding the second point.

    @return Whether or not the box expanded
    */
    inline bool addPoint(const glm::detail::tvec3<T>& point) {
        bool changed = false;

        for(unsigned int coord = 0; coord < 3; coord++) {
            if(point[coord] < m_min[coord]) {
                m_min[coord] = point[coord];
                changed = true;
            }

            if(point[coord] > m_max[coord]) {
                m_max[coord] = point[coord];
                changed = true;
            }
        }

        return changed;
    }

    /**
    Constrains the bounds of the other box within the bounds of this box.
    Modifies the passed in box so be sure to create a copy of it if you don't want the original changed.

    @return Whether or not the box dimensions changed.
    */
    inline bool constrain(Box& other) const {
        bool changed = false;

        for(unsigned int coord = 0; coord < 3; coord++) {
            if(other.m_min[coord] < m_min[coord]) {
                other.m_min[coord] = m_min[coord];
                changed = true;
            }

            if(other.m_max[coord] > m_max[coord]) {
                other.m_max[coord] = m_max[coord];
                changed = true;
            }
        }

        return changed;
    }

    /**
    Constrains a point within the bounds of the box.
    Modifies the passed in point so be sure to create a copy of it if you don't want the original changed.

    @return Whether or not the point changed.
    */
    inline bool constrain(glm::detail::tvec3<T>& point) const {
        bool changed = false;

        for(unsigned int coord = 0; coord < 3; coord++) {
            if(point[coord] < m_min[coord]) {
                point[coord] = m_min[coord];
                changed = true;
            }

            if(point[coord] > m_max[coord]) {
                point[coord] = m_max[coord];
                changed = true;
            }
        }

        return changed;
    }

    /**
    Checks if this and another box intersect
    */
    inline bool intersects(const Box& other) const {
        for(unsigned int coord = 0; coord < 3; coord++) {
            if(other.m_min[coord] > m_max[coord] || other.m_max[coord] < m_min[coord]) {
                return false;
            }
        }

        return true;
    }

    /**
    Checks if the box contains a point
    */
    inline bool intersects(const glm::detail::tvec3<T>& point) const {
        for(unsigned int coord = 0; coord < 3; coord++) {
            if(point[coord] > m_max[coord] || point[coord] < m_min[coord]) {
                return false;
            }
        }

        return true;
    }

    /**
    Gets the coords of closest corner on the box to a point
    */
    inline glm::detail::tvec3<T> getClosestCorner(const glm::detail::tvec3<T>& vector) const {
        glm::detail::tvec3<T> res;

        //find closest box point
        for(unsigned int coord = 0; coord < 3; coord++) {
            res[coord] = glm::abs(vector[coord] - m_min[coord]) < glm::abs(vector[coord] - m_max[coord]) ? m_min[coord] : m_max[coord];
        }

        return res;
    }

    /**
    Returns a chess metric distance from a point to the box.
    */
    inline T getChessDistance(const glm::detail::tvec3<T>& vector) const {
        if(intersects(vector)) {
            return (T)0;
        }
        else {
            return chessDistance(vector, getClosestCorner(vector));
        }
    }

    /**
    Offsets the bounds in a box by a vector
    */
    inline Box operator+(const glm::detail::tvec3<T>& vector) const {    
        return Box(m_min + vector, m_max + vector);
    }

    /**
    Offsets the bounds in a box by a vector
    */
    inline Box& operator+=(const glm::detail::tvec3<T>& vector) {
        m_min += vector;
        m_max += vector;

        return *this;
    }

    /**
    Offsets the bounds in a box by a vector
    */
    inline Box operator-(const glm::detail::tvec3<T>& vector) const {    
        return Box(m_min - vector, m_max - vector);
    }

    /**
    Offsets the bounds in a box by a vector
    */
    inline Box& operator-=(const glm::detail::tvec3<T>& vector) {
        m_min -= vector;
        m_max -= vector;

        return *this;
    }

    //TODO: add any other needed functions

    ///The bottom, left, near corner
    glm::detail::tvec3<T> m_min;

    ///The top, right, far corner
    glm::detail::tvec3<T> m_max;
};

#endif