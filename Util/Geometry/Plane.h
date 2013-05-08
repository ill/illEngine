#ifndef ILL_PLANE_H__
#define ILL_PLANE_H__

#include <glm/glm.hpp>

/**
Plane represented by equation ax + by + cz + d = 0

(a,b,c) is a 3D vector pointing in the normal direction of the plane
d is the negative distance of the plane from the origin

Useful in view frustum culling and collision detection when not relying on a physics engine
*/
template <typename T = glm::mediump_float>
struct Plane {
    inline Plane() {}
    
    /**
    Copy constructor
    */
    inline Plane(const Plane& other) 
        : m_normal(other.m_normal),
        m_distance(other.m_distance)
    {}

    /**
    Constructs plane from a normal vector and distance from origin
    @param normal The normal vector
    @param distance The distance from origin
    */
    inline Plane(const glm::detail::tvec3<T> normal, const T distance)
        : m_normal(normal),
        m_distance(distance)
    {}

    /**
    Constructs a plane from just a 4 element vector.
    @param vec A 4 element vector.  The x,y,z portion is the normal
        and the w portion is the distance
    */
    inline Plane(const glm::detail::tvec4<T> vec)
        : m_normal(vec),
        m_distance(vec.w)
    {}

    inline ~Plane() {}

    /**
    Normalizes the plane.
    Just don't let the normal vector have a length of zero.
    */
    inline Plane normalize() const {
        Plane res;

        T denom = 1 / glm::length(m_normal);

        res.m_normal = m_normal * denom;
        res.m_distance = m_distance * denom;

        return res;
    }

    /**
    Transforms a plane.
    The transform passed in should be the inverse transpose
    or be an orthogonal matrix.

    http://stackoverflow.com/questions/7685495/transforming-a-3d-plane-by-4x4-matrix
    */
    inline Plane transform(glm::detail::tmat4x4<T> xform) const {
        Plane res;

        //transforming the normal is simple
        res.m_normal = glm::detail::tvec3<T>(xform * glm::detail::tvec4<T>(m_normal, (T)0));
        
        //first find a point on the plane and transform that
        //glm::detail::tvec3<T> point = glm::detail::tvec3<T>(xform * glm::detail::tvec4<T>(m_normal * m_distance, (T)1));

        //then get the distance again
        //res.m_distance = -glm::dot(point, res.m_normal);

        //and this is based on PhysX source code.  Seems a bit more simple and efficient.
        //This doesn't seem to work...
        res.m_distance = m_distance - glm::dot(getTransformPosition(xform), res.m_normal);

        return res;
    }

    /**
    Returns the distance from from the plane to a point.
    */
    inline T distance(const glm::detail::tvec3<T>& point) const {
        return glm::dot(m_normal, point) + m_distance;
    }

    /**
    Finds which side of the plane a point is on.
    @param point The point

    @return true if On the plane or on the side of the plane the normal is pointing,
        false if On the other side.
    */
    inline bool pointOnSide(const glm::detail::tvec3<T>& point) const {
        return distance(point) >= (T) 0;
    }

    /**
    Finds the intersection of a line segment and a plane.
    @param ptA One end of the line segment.
    @param ptB The other end of the line segment.
    @param resultDestination Where the resulting intesection point will be written to if there was an intersection.

    @return Whether or not there was an intersection.
    Check this before using the value written to resultDestination or else you'll have undefined results.

    @see http://paulbourke.net/geometry/planeline/
    */
    inline bool lineIntersection(const glm::detail::tvec3<T>& ptA, const glm::detail::tvec3<T>& ptB, glm::detail::tvec3<T>& resultDestination) const {
        glm::detail::tvec3<T> vec = m_normal * (ptA - ptB);      
        T denom = vec.x + vec.y + vec.z;

        if(denom == (T)0) {
            //line is perpendicular to plane
            return false;
        }

        vec = m_normal * ptA;
        T dist = (vec.x + vec.y + vec.z + m_distance) / denom;

        if(dist >= (T)0 && dist <= (T)1) {
            resultDestination = ptA + (ptB - ptA) * dist;
            return true;
        }
        else {
            return false;
        }
    }

    ///The plane's normal vector
    glm::detail::tvec3<T> m_normal;

    ///The plane's distance from the origin
    T m_distance;
};

/**
Finds the intersection point between 3 planes
@see http://www.cgafaq.info/wiki/Intersection_of_three_planes
*/
template <typename T>
inline glm::detail::tvec3<T> planeIntersection(const Plane<T>& planeA, const Plane<T>& planeB, const Plane<T>& planeC) {
    glm::detail::tvec3<T> bcCross = glm::cross(planeB.m_normal, planeC.m_normal);
    T denom = glm::dot(planeA.m_normal, bcCross);

    if (denom == 0) {
        // All kinds of errors. Should never happen unless planes are parallel
        return glm::detail::tvec3<T>(0);
    }
    else {
        return (-planeA.m_distance * bcCross 
            - planeB.m_distance * glm::cross(planeC.m_normal, planeA.m_normal) 
            - planeC.m_distance * glm::cross(planeA.m_normal, planeB.m_normal)) / denom;
    }
}

#endif