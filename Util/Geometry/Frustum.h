#ifndef ILL_FRUSTUM_H__
#define ILL_FRUSTUM_H__

#include <stdint.h>
#include <glm/glm.hpp>

#include "Plane.h"
#include "Box.h"

/**
Some helpers for accessing the corner point indeces array that make up the frustum.
NBL means Near Bottom Left point of the frustum for example.
*/
const uint8_t FRUSTUM_NBL = 0;
const uint8_t FRUSTUM_NBR = 1;
const uint8_t FRUSTUM_NTL = 3;
const uint8_t FRUSTUM_NTR = 2;
const uint8_t FRUSTUM_FBL = 4;
const uint8_t FRUSTUM_FBR = 5;
const uint8_t FRUSTUM_FTL = 7;
const uint8_t FRUSTUM_FTR = 6;

const uint8_t FRUSTUM_NUM_POINTS = 8;
const uint8_t FRUSTUM_NUM_EDGES = 12;

/**
Static list of point indeces that make up the edges of a view frustum.
*/
const uint8_t FRUSTUM_EDGE_LIST[FRUSTUM_NUM_EDGES][2] = {
    //near side edge loop
    {FRUSTUM_NBL, FRUSTUM_NBR},
    {FRUSTUM_NBR, FRUSTUM_NTR},
    {FRUSTUM_NTR, FRUSTUM_NTL},
    {FRUSTUM_NTL, FRUSTUM_NBL},

    //far side edge loop
    {FRUSTUM_FBL, FRUSTUM_FBR},
    {FRUSTUM_FBR, FRUSTUM_FTR},
    {FRUSTUM_FTR, FRUSTUM_FTL},
    {FRUSTUM_FTL, FRUSTUM_FBL},

    //edges joining near and far
    {FRUSTUM_NBL, FRUSTUM_FBL},
    {FRUSTUM_NBR, FRUSTUM_FBR},
    {FRUSTUM_NTR, FRUSTUM_FTR},
    {FRUSTUM_NTL, FRUSTUM_FTL}
};

/**
A frustum is a pyramid with the top cut off.  It's typically used to represent what's visible in a 3D camera view.
The top of the pyramid is the near plane, and the bottom is the far plane.
*/
template <typename T = glm::mediump_float>
struct Frustum {
    inline Frustum() {}

    /**
    Constructs the view frustum given a canonical matrix
    */
    inline Frustum(const glm::detail::tmat4x4<T>& canonicalMatrix) {
        set(canonicalMatrix);
    }

    /**
    Sets the view frutum from a canonical matrix
    */
    void set(const glm::detail::tmat4x4<T>& canonicalMatrix) {
        //compute planes (Originally based on: http://www.lighthouse3d.com/tutorials/view-frustum-culling/clip-space-approach-implementation-details/)
        m_left.m_normal = glm::detail::tvec3<T>(    canonicalMatrix[0][3] + canonicalMatrix[0][0], 
                                                    canonicalMatrix[1][3] + canonicalMatrix[1][0], 
                                                    canonicalMatrix[2][3] + canonicalMatrix[2][0]);
        m_left.m_distance =                         canonicalMatrix[3][3] + canonicalMatrix[3][0];
        m_left = m_left.normalize();

        m_right.m_normal = glm::detail::tvec3<T>(   canonicalMatrix[0][3] - canonicalMatrix[0][0],
                                                    canonicalMatrix[1][3] - canonicalMatrix[1][0],
                                                    canonicalMatrix[2][3] - canonicalMatrix[2][0]);
        m_right.m_distance =                        canonicalMatrix[3][3] - canonicalMatrix[3][0];
        m_right = m_right.normalize();

        m_bottom.m_normal = glm::detail::tvec3<T>(  canonicalMatrix[0][3] + canonicalMatrix[0][1],
                                                    canonicalMatrix[1][3] + canonicalMatrix[1][1],
                                                    canonicalMatrix[2][3] + canonicalMatrix[2][1]);
        m_bottom.m_distance =                       canonicalMatrix[3][3] + canonicalMatrix[3][1];
        m_bottom = m_bottom.normalize();

        m_top.m_normal = glm::detail::tvec3<T>(     canonicalMatrix[0][3] - canonicalMatrix[0][1],
                                                    canonicalMatrix[1][3] - canonicalMatrix[1][1],
                                                    canonicalMatrix[2][3] - canonicalMatrix[2][1]);
        m_top.m_distance =                          canonicalMatrix[3][3] - canonicalMatrix[3][1];
        m_top = m_top.normalize();

        m_near.m_normal = glm::detail::tvec3<T>(    canonicalMatrix[0][3] + canonicalMatrix[0][2],
                                                    canonicalMatrix[1][3] + canonicalMatrix[1][2],
                                                    canonicalMatrix[2][3] + canonicalMatrix[2][2]);
        m_near.m_distance =                         canonicalMatrix[3][3] + canonicalMatrix[3][2];
        m_near = m_near.normalize();

        m_far.m_normal = glm::detail::tvec3<T>(     canonicalMatrix[0][3] - canonicalMatrix[0][2],
                                                    canonicalMatrix[1][3] - canonicalMatrix[1][2],
                                                    canonicalMatrix[2][3] - canonicalMatrix[2][2]);
        m_far.m_distance =                          canonicalMatrix[3][3] - canonicalMatrix[3][2];
        m_far = m_far.normalize();

        //compute frustum corners which are pretty useful sometimes
        m_points[FRUSTUM_NBL] = planeIntersection(m_near, m_bottom, m_left);
        m_points[FRUSTUM_NTL] = planeIntersection(m_near, m_top, m_left);
        m_points[FRUSTUM_NBR] = planeIntersection(m_near, m_bottom, m_right);
        m_points[FRUSTUM_NTR] = planeIntersection(m_near, m_top, m_right);
        m_points[FRUSTUM_FBL] = planeIntersection(m_far, m_bottom, m_left);
        m_points[FRUSTUM_FTL] = planeIntersection(m_far, m_top, m_left);
        m_points[FRUSTUM_FBR] = planeIntersection(m_far, m_bottom, m_right);
        m_points[FRUSTUM_FTR] = planeIntersection(m_far, m_top, m_right);

        //compute frustum positions
        m_nearPoint = (m_points[FRUSTUM_NBL] + m_points[FRUSTUM_NTL] + m_points[FRUSTUM_NBR] + m_points[FRUSTUM_NTR]) / (T)4;
        m_farPoint = (m_points[FRUSTUM_FBL] + m_points[FRUSTUM_FTL] + m_points[FRUSTUM_FBR] + m_points[FRUSTUM_FTR]) / (T)4;

        //view frustum bounding box which is useful for broad collision detection
        m_bounds = Box<T>(m_points[FRUSTUM_NBL]);
        m_bounds.addPoint(m_points[FRUSTUM_NTL]);
        m_bounds.addPoint(m_points[FRUSTUM_NBR]);
        m_bounds.addPoint(m_points[FRUSTUM_NTR]);
        m_bounds.addPoint(m_points[FRUSTUM_FBL]);
        m_bounds.addPoint(m_points[FRUSTUM_FTL]);
        m_bounds.addPoint(m_points[FRUSTUM_FBR]);
        m_bounds.addPoint(m_points[FRUSTUM_FTR]);

        //near tip point
        m_nearTipPoint = planeIntersection(m_top, m_bottom, m_left);

        //find biggest distance for radius in case frustum isn't symmetrical for some reason
        m_radius = distance2(m_nearTipPoint, m_points[FRUSTUM_FBL]);
        m_radius = glm::max(m_radius, distance2(m_nearTipPoint, m_points[FRUSTUM_FBR]));
        m_radius = glm::max(m_radius, distance2(m_nearTipPoint, m_points[FRUSTUM_FTL]));
        m_radius = glm::max(m_radius, distance2(m_nearTipPoint, m_points[FRUSTUM_FTR]));

        m_radius = glm::sqrt(m_radius);

        //frustum direction
        m_direction = glm::normalize(m_farPoint - m_nearPoint);

        //far tip point
        m_farTipPoint = m_nearTipPoint + m_direction * m_radius;

        //directed bounds
        glm::vec3 direction = glm::sign(m_direction);

        for(unsigned int dimension = 0; dimension < 3; dimension++) {
            if(direction[dimension] >= 0) {
                m_directedBounds.m_min[dimension] = m_bounds.m_min[dimension];
                m_directedBounds.m_max[dimension] = m_bounds.m_max[dimension];
            }
            else {
                m_directedBounds.m_min[dimension] = m_bounds.m_max[dimension];
                m_directedBounds.m_max[dimension] = m_bounds.m_min[dimension];
            }
        }
    }

    //the frustum planes
    Plane<T> m_left;
    Plane<T> m_right;
    Plane<T> m_top;
    Plane<T> m_bottom;
    Plane<T> m_near;
    Plane<T> m_far;

    //the corner positions in 3D space
    glm::detail::tvec3<T> m_points[8];

    //the positions of middles of planes
    glm::detail::tvec3<T> m_nearPoint;
    glm::detail::tvec3<T> m_farPoint;

    //the position of the pyramid tip were it not cut off, basically the origin of the frustum
    glm::detail::tvec3<T> m_nearTipPoint;

    //the position of the point in the direction of the frustum that is the radius distance away from the near tip point
    glm::detail::tvec3<T> m_farTipPoint;

    //the direction the frustum is pointing
    glm::detail::tvec3<T> m_direction;

    //the radius of the frustum, found by finding the longest distance from the near tip point to one of the farthest point on the far plane
    T m_radius;

    //axis aligned bounding box of view frustum
    Box<T> m_bounds;

    //axis aligned bounding box of view frustum but isn't normalized, it's oriented in the direction of the frustum
    Box<T> m_directedBounds;
};

#endif