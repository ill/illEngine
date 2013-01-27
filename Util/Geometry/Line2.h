#ifndef ILL_LINE2_H_
#define ILL_LINE2_H_

#include <glm/glm.hpp>
#include "Util/Geometry/geomUtil.h"

template <typename T = glm::mediump_float>
struct Line2 {
    Line2() {}

    Line2(const glm::detail::tvec2<T>& ptA, const glm::detail::tvec2<T>& ptB) {
        m_point[0] = ptA;
        m_point[1] = ptB;
    }

    /**
    Finds the y intercept of this line.
    Never use this on vertical lines or it will have a divide by zero errror.

    @param x The x coordinate at which to find the y intercept

    @return The y intercept
    */
    template <typename T>
    inline T yIntercept(T x) {   
        return lineInterceptY(m_point[0], m_point[1], x);
    }

    /**
    Finds the x intercept of this line.
    Never use this on horizontal lines or it will have a divide by zero errror.

    @param y The y coordinate at which to find the x intercept

    @return The x intercept
    */
    template <typename T>
    inline T xIntercept(T y) {
        return lineInterceptX(m_point[0], m_point[1], y);
    }
    
    glm::detail::tvec2<T> m_point[2];
};

#endif