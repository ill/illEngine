//
//  geomUtil.h
//  Alien Scum
//
//  Created by Ilya Seletsky on 3/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef ILL_GEOMUTIL_H__
#define ILL_GEOMUTIL_H__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Util/Geometry/Box.h"

#include "Util/util.h"

/**
Used by computePosition
*/
enum RectPosition {
    RP_LEFT,          ///< Relative to the left side
    RP_CENTER,        ///< Relative to the middle
    RP_RIGHT          ///< Relative to the right side
};

/**
Computes a pixel position inside some pixel sized rectangle given coordinates that are a factor of the size of the rectangle.
This is usually useful for figuring out locations of HUD elements on the screen while at the same time aligning them correctly
relative to whatever horizontal edge they are on regardless of aspect ratio and screen resolution.

@tparam POS The type for the input position, usually glm::mediump_float AKA float
@tparam RECT The type for the rectangle dimensions and position.  If working with pixels, would be unsigned int, but can be float also if working with some arbitrary box size.

@param position A position passed in as floats.  These positions are relative to the height of the rectangle.
1.0 is the full height, 0.5 is half the height, -.25 is 1/4 of the height but outside.   
@param rectDimensions The dimensions of the rectangle, usually in pixels.
@param relativePosition Where relative to the rectangle is the x coordinate in the position parameter, left, center, or middle.
If left is passed in, the returned position is the x distance to the left of the left edge of the rectangle.
If center is passed in, the returned position is the x distance to the left of the center of the rectangle.
If right is passed in, the returned position is the x distance to the left of the right edge of the rectangle.

@return A position in pixels within the rectangle.
Some examples:
If given (0.0, 0.0) with relativeHorzPosition SP_LEFT the resulting point would be 0, 0.
If given (1.0, 1.0) with relativeHorzPosition SP_LEFT and dimensions 800x600 the resulting point would be at (600,600)
If given (0.0, 0.5) with relativeHorzPosition SP_CENTER and dimensions 800x600 the resulting point would be at (400,300)
*/
template<typename POS, typename RECT>
inline glm::detail::tvec2<RECT> computePosition(glm::detail::tvec2<POS> position, glm::detail::tvec2<RECT> rectDimensions, RectPosition relativeHorzPosition) {
    glm::detail::tvec2<RECT> res;

    res.y = (RECT) (position.y * rectDimensions.y);

    switch(relativeHorzPosition) {
    case RP_LEFT:
        res.x = (RECT) (position.x * rectDimensions.y);
        break;

    case RP_CENTER:
        res.x = (RECT) (rectDimensions.x * 0.5f + position.x * rectDimensions.y);
        break;

    case RP_RIGHT:
        res.x = (RECT) (rectDimensions.x - position.x * rectDimensions.y);
        break;
    }

    return res;
}

/**
Computes the pixel dimensions of some rectangle given some dimensions that are a factor of the rectangle's height.
This is useful for figuring out the dimensions of HUD elements similar to how computePosition works where it takes aspect ratio into account.

@tparam POS The type for the input position, usually glm::mediump_float AKA float
@tparam RECT The type for the rectangle dimensions and position.  If working with pixels, would be unsigned int, but can be float also if working with some arbitrary box size.

@param dimensions Some dimensions passed in as floats.  These dimensions are relative to the height of the rectangle.
1.0 is the full height, 0.5 is half the height, .25 is 1/4 of the height.
@param rectHeight The height of the rectangle.

@return Some dimensions relative to the height of the rectangle.
*/
template<typename POS, typename RECT>
inline glm::detail::tvec2<RECT> computeDimensions(glm::detail::tvec2<POS> dimensions, RECT rectHeight) {
    glm::detail::tvec2<RECT> res;

    res.x = (RECT) (dimensions.x * rectHeight);
    res.y = (RECT) (dimensions.y * rectHeight);

    return res;
}

/**
GLM seems to not have this useful function.
Typecasts a vector from 1 type to another, useful for assigning a vector of ints to a vector of floats or something.

@tparam FROM type of vector casting from
@tparam TO type of vector casting to
*/
template <typename FROM, typename TO>
inline glm::detail::tvec3<TO> vec3cast(const glm::detail::tvec3<FROM>& value) {
    glm::detail::tvec3<TO> res;

    for(unsigned int coord = 0; coord < 3; coord++) {
        res[coord] = (TO) value[coord];
    }

    return res;
}

template <typename T>
inline glm::detail::tvec3<T> signO(const glm::detail::tvec3<T>& value) {
    glm::detail::tvec3<T> res;

    for(unsigned int coord = 0; coord < 3; coord++) {
        res[coord] = signO(value[coord]);
    }

    return res;
}

/**
A container that stores things like quaternions or vectors and allows them to be LERPED

TODO: document a bit better later
*/
template <typename GEOM_TYPE, typename T = glm::mediump_float>
struct LerpTransform {
    GEOM_TYPE m_base;
    GEOM_TYPE m_destination;
    T m_t;
    T m_delta;

    LerpTransform() {}

    LerpTransform(const GEOM_TYPE& base, const GEOM_TYPE& destination, const T& delta, const T& t = 0.0f) 
        : m_base(base),
        m_destination(destination),
        m_t(t),
        m_delta(delta)
    {}

    inline GEOM_TYPE mix() {      //TODO: for some reason glm's mix doesn't seem to clamp between 0 and 1
        if(m_t <= (T)0) {
            return m_base;
        }
        else if(m_t >= (T)1) {
            return m_destination;
        }
        else {      
            return glm::shortMix(m_base, m_destination, m_t);
        }
    }

    inline void reset(const GEOM_TYPE& destination, const T& delta) {
        m_base = mix();

        m_delta = delta;
        m_destination = destination;

        m_t = (T)0;
    }

    inline void update(float timeStep) {
        m_t += m_delta * timeStep;
    }
};

/**
Converts a quaternion to a direction vector.
This is super simple BTW, it's just a convenience function for rotating an unrotated vector by the quaternion
*/
template <typename T>
inline glm::detail::tvec3<T> quatToDirection(const glm::detail::tquat<T>& value) {
    glm::detail::tvec3<T> val = glm::rotate(value, glm::detail::tvec3<T>((T)0, (T)0, (T)-1));
    return val;
}

template <typename T>
inline glm::detail::tvec3<T> mat3ToDirection(const glm::detail::tmat3x3<T>& value) {
    glm::detail::tvec3<T> val = value * glm::detail::tvec3<T>((T)0, (T)0, (T)1);
    return val;
}

/**
Converts a normalized direction vector and up vector to a quaternion
*/
template <typename T>
inline glm::detail::tmat3x3<T> directionToMat3(const glm::detail::tvec3<T>& direction, const glm::detail::tvec3<T>& up = glm::detail::tvec3<T>((T)0, (T)1, (T)0)) {
    glm::vec3 upProjection = glm::normalize(up - direction * glm::dot(up, direction));
    glm::vec3 rightProjection = glm::cross(direction, upProjection);     //TODO: make sure this cross product is correct for a right handed coord system

    glm::detail::tmat3x3<T> retVal = glm::detail::tmat3x3<T>(rightProjection, upProjection, -direction);

    return retVal;
}

template <typename T>
inline glm::detail::tquat<T> directionToQuat(const glm::detail::tvec3<T>& direction, const glm::detail::tvec3<T>& up = glm::detail::tvec3<T>((T)0, (T)1, (T)0)) {   
    return glm::quat_cast(directionToMat3(direction, up));
}

/**
Returns the squared distance between 2 points.  This avoids a square root operation when one isn't necessary.  You can easily compare squared distances.
*/
template <typename T>
inline T distance2(const glm::detail::tvec3<T>& v1, const glm::detail::tvec3<T>& v2) {
    return (v1.x - v2.x) * (v1.x - v2.x) 
        + (v1.y - v2.y) * (v1.y - v2.y)
        + (v1.z - v2.z) * (v1.z - v2.z);
}

/**
Safely normalizes a vector so if the magnitude is zero it stays as a zero vector and doesn't create NaN values
*/
template <typename T>
inline glm::detail::tvec3<T> safeNormalize(const glm::detail::tvec3<T>& vec) {
    if(vec.x == (T)0 && vec.y == (T)0 && vec.z == (T)0) {
        return vec;
    }
    else {
        return glm::normalize(vec);
    }
}

/**
Computes a chess metric distance instead of the normal euclidian metric distance.  This makes sense when working with grids.
*/
template <typename T>
inline T chessDistance(const glm::detail::tvec3<T>& v1, const glm::detail::tvec3<T>& v2) {
    T res = glm::max(glm::abs(v1.x - v2.x), glm::abs(v1.y - v2.y));
    res = glm::max(res, glm::abs(v1.z - v2.z));

    return res;
}

/**
This returns what the perpendicular z component would be of 2 2D vectors.
2D cross product function is missing from glm.

@param v1 Vector 1
@param v2 Vector 2

@return 0 if vectors are colinear
positive value if vectors make a counter clockwise turn
negative value if vectors make a clockwise turn
*/
template <typename T>
inline T cross(const glm::detail::tvec2<T>& v1, const glm::detail::tvec2<T>& v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

/**
Returns the 2D intersection between 2 lines, each defined by 2 points each.

Based on http://paulbourke.net/geometry/lineline2d/

@param pt1 Line 1 point 1
@param pt2 Line 1 point 2
@param pt3 Line 2 point 1
@param pt4 Line 2 point 2
@param destination Where the intersection point will be written if there was an intersection

@return Whether or not there actually is an intersection.  You need to check this value before using what's written to destination
or at least assert it if you know for sure there should always be an intersection in some case.
*/
template <typename T>
inline bool lineIntersection(const glm::detail::tvec2<T>& pt1, const glm::detail::tvec2<T>& pt2, const glm::detail::tvec2<T>& pt3, const glm::detail::tvec2<T>& pt4, glm::detail::tvec2<T>& destination) {      
    T multiplier = ((pt4.y - pt3.y) * (pt2.x - pt1.x)) - ((pt4.x - pt3.x) * (pt2.y - pt1.y));

    if(multiplier == (T) 0) {
        return false;
    }

    multiplier = (((pt4.x - pt3.x) * (pt1.y - pt3.y)) - ((pt4.y - pt3.y) * (pt1.x - pt3.x))) / multiplier;

    destination = pt1 + multiplier * (pt2 - pt1);

    return true;
}

/**
Finds the y intercept of a line defined by 2 points.
Never use this on vertical lines or it will have a divide by zero errror.

@param pt1 Line point 1
@param pt2 Line point 2
@param x The x coordinate at which to find the y intercept

@return The y intercept
*/
template <typename T>
inline T lineInterceptY(const glm::detail::tvec2<T>& pt1, const glm::detail::tvec2<T>& pt2, T x) {   
    return pt2.y + ((pt2.y - pt1.y) / (pt2.x - pt1.x)) * (x - pt2.x);
}

/**
Finds the x intercept of a line defined by 2 points.
Never use this on horizontal lines or it will have a divide by zero errror.

@param pt1 Line point 1
@param pt2 Line point 2
@param y The y coordinate at which to find the x intercept

@return The x intercept
*/
template <typename T>
inline T lineInterceptX(const glm::detail::tvec2<T>& pt1, const glm::detail::tvec2<T>& pt2, T y) {
    return pt2.x + ((pt2.x - pt1.x) / (pt2.y - pt1.y)) * (y - pt2.y);
}

/**
Returns a vector of dimensions sorted by magnitude in the given vector.
X is dimension 0, Y is dimension 1, Z is dimension 2.
Good for use in for loops to index into other vectors.

For example, given vector (5, 9, -2) it returns (2, 0, 1).
*/
template <typename T>
inline glm::detail::tvec3<uint8_t> sortDimensions(glm::detail::tvec3<T> vec) {
    //I'd probably find a much more generic way to do this if this was arbitrary dimensional

    glm::detail::tvec3<uint8_t> res;
    vec = glm::abs(vec);

    T maxMagnitude;
    T secMagnitude;

    res[2] = 2;

    if(vec.x > vec.y) {
        res[0] = 0;
        res[1] = 1;

        maxMagnitude = vec.x;
        secMagnitude = vec.y;
    }
    else {
        res[0] = 1;
        res[1] = 0;

        maxMagnitude = vec.y;
        secMagnitude = vec.x;
    }

    if(vec.z > maxMagnitude) {
        res[2] = res[1];
        res[1] = res[0];
        res[0] = 2;
    }
    else if(vec.z > secMagnitude) {
        res[2] = res[1];
        res[1] = 2;
    }

    return res;
}

/**
Generates a random vector
TODO: document better
*/
/*template <typename T = glm::mediump_float>
inline glm::detail::tvec3<T> randomVec(const glm::detail::tvec3<T>& minRange, const glm::detail::tvec3<T>& maxRange) {
glm::detail::tvec3<T> res(0.0f);

for(int i = 0; i < 3; i++) {
}

return res;
}*/

/*
vx = random(-range, +range);
vy = random(-range, +range);
vz = sqrt(1.0f - (vx * vx + vy * vy)); // good
vz = 1.0f;*/

template <typename T>
inline glm::detail::tvec3<T> getTransformPosition(const glm::detail::tmat4x4<T>& transform) {
    return glm::detail::tvec3<T>(transform[3][0], transform[3][1], transform[3][2]);
}

template <typename T>
inline glm::detail::tmat4x4<T> setTransformPosition(const glm::detail::tmat4x4<T>& transform, const glm::detail::tvec3<T>& position) {
    glm::detail::tmat4x4<T> res = transform;

    res[3][0] = position.x;
    res[3][1] = position.y;
    res[3][2] = position.z;

    return res;
}

template <typename T>
inline glm::detail::tvec3<T> getTransformScale(const glm::detail::tmat4x4<T>& transform) {
    return glm::detail::tvec3<T>(transform[0][0], transform[1][1], transform[2][2]);
}

/**
Creates a 4x4 transformation matrix given a position
*/
template <typename T>
inline glm::detail::tmat4x4<T> createTransform(const glm::detail::tvec3<T>& position) {
    glm::detail::tmat4x4<T> res;
    res = glm::translate(res, position);

    return res;
}

//TODO: make a version of this func that takes a rotation matrix
/**
Creates a 4x4 transformation matrix given a position and rotation
*/
template <typename T>
inline glm::detail::tmat4x4<T> createTransform(const glm::detail::tvec3<T>& position,  
    const glm::detail::tquat<T>& rotation) {
        glm::detail::tmat4x4<T> res = createTransform(position);
        res = res * glm::mat4_cast(rotation);

        return res;
}

//TODO: remplate this to take either a mat3x3 or a quat
template <typename T>
inline glm::detail::tmat4x4<T> createTransform(const glm::detail::tvec3<T>& position,  
    const glm::detail::tmat3x3<T>& rotation) {
        glm::detail::tmat4x4<T> res = createTransform(position);
        res = res * glm::detail::tmat4x4<T>(rotation);

        return res;
}

/**
Creates a 4x4 transformation matrix given a position, rotation, and scale
*/
template <typename T>
inline glm::detail::tmat4x4<T> createTransform(const glm::detail::tvec3<T>& position, 
    const glm::detail::tquat<T>& rotation,
    const glm::detail::tvec3<T>& scale) {
        glm::detail::tmat4x4<T> res = createTransform(position, rotation);
        res = glm::scale(res, scale);

        return res;
}

/**
Read docs for dampen under util.h.  Only this works on the components of a vector.
*/
template <typename T>
inline glm::detail::tvec3<T> dampenVec(glm::detail::tvec3<T> value, const glm::detail::tvec3<T>& dampFactor, const glm::detail::tvec3<T>& dampTarget) {   
    for(int i = 0; i < 3; i++) {
        value[i] = dampen(value[i], dampFactor[i], dampTarget[i]);
    }

    return value;
}

/**
Read docs for highPassFilter under util.h.  Only this works on the components of a vector.
*/
template <typename T>
inline glm::detail::tvec3<T> highPassFilterVec(glm::detail::tvec3<T> value, const glm::detail::tvec3<T>& previousValue, const glm::detail::tvec3<T>& filterFactor) {
    for(int i = 0; i < 3; i++) {
        value[i] = highPassFilter(value[i], previousValue[i], filterFactor[i]);
    }

    return value;
}

/**
Read docs for drift under util.h.  Only this works on the components of a vector.
*/
template <typename T>
inline glm::detail::tvec3<T> driftVec(glm::detail::tvec3<T> value, const glm::detail::tvec3<T>& otherValue, const glm::detail::tvec3<T>& destination, const glm::detail::tvec3<T>& falloffFactor) {
    for(int i = 0; i < 3; i++) {
        value[i] = drift(value[i], otherValue[i], destination[i], falloffFactor[i]);
    }

    return value;
}

/**
Read docs for dampen under util.h.  Only this works on the components of a quaternion.
*/
template <typename T>
inline glm::detail::tquat<T> dampenQuat(glm::detail::tquat<T> value, const glm::detail::tquat<T>& dampFactor, const glm::detail::tquat<T>& dampTarget) {   
    for(int i = 0; i < 4; i++) {
        value[i] = dampen(value[i], dampFactor[i], dampTarget[i]);
    }

    return value;
}

/**
Read docs for grid under util.h.  Only this works on the components of a vector.
*/
template <typename T, typename R>
inline glm::detail::tvec3<R> gridVec(const glm::detail::tvec3<T>& value, const glm::detail::tvec3<T>& dimensions) {
    glm::detail::tvec3<R> res;
    
    for(int i = 0; i < 3; i++) {
        res[i] = grid<T, R>(value[i], dimensions[i]);
    }

    return res;
}

template <typename T>
inline glm::detail::tvec3<T> parseVector(std::istream& openFile) {
    glm::detail::tvec3<T> res;

    openFile >> res.x;
    openFile >> res.y;
    openFile >> res.z;

    return res;
}

template <typename T>
inline Box<T> parseBox(std::istream& openFile) {
    Box<T> res;

    res.m_min = parseVector<T>(openFile);
    res.m_max = parseVector<T>(openFile);

    return res;
}

template <typename T>
inline glm::detail::tquat<T> parseQuaternion(std::istream& openFile) {
    glm::detail::tquat<T> res;

    openFile >> res.x;
    openFile >> res.y;
    openFile >> res.z;
    openFile >> res.w;

    return res;
}

template <typename T>
inline glm::detail::tmat4x4<T> parseMat4(std::istream& openFile) {
    glm::detail::tmat4x4<T> res;

    for(unsigned int row = 0; row < 4; row++) {
        for(unsigned int column = 0; column < 4; column++) {
            openFile >> res[column][row];
        }
    }

    return res;
}

#endif
