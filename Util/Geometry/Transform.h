#ifndef ILL_TRANSFORM_H__
#define ILL_TRANSFORM_H__

#include <glm/gtc/quaternion.hpp>
#include "geomUtil.h"

/**
Transforms are usually stored as 4x4 matrices but in this form they are easier to interpolate.
This is 15 floats while a 4x4 matrix is 16 floats so this isn't much of an improvement on memory.
You end up still needing to convert it back to a matrix before using it for rendering so just use matrices pretty much most of the time.
*/
template <typename T = glm::mediump_float>
struct Transform {
    Transform()
		: m_scale(1.0f)
	{}

    Transform(const glm::detail::tvec3<T>& position, 
        const glm::detail::tquat<T>& rotation, 
        const glm::detail::tvec3<T>& scale = glm::detail::tvec3<T>((T)1))
        : m_position(position),
        m_rotation(rotation),
        m_scale(scale)
    {}

    Transform(const glm::mat4& transform) {
        set(transform);
    }

    //TODO: add more constructors as needed, such as from a 4x4 matrix, a 3x3 matrix for rotation instead of a quaternion, etc...

    inline Transform interpolate(const Transform& other, T delta) const {
        Transform res;

        res.m_position = glm::mix(m_position, other.m_position, delta);
        res.m_rotation = glm::shortMix(m_rotation, other.m_rotation, delta);
        res.m_scale = glm::mix(m_scale, other.m_scale, delta);

        return res;
    }

    inline glm::detail::tmat4x4<T> getMatrix() const {
		return glm::scale(glm::translate(m_position) * glm::mat4_cast(m_rotation), m_scale);
    }

    inline void set(const glm::mat4& transform) {
        m_position = getTransformPosition(transform);
        getTransformRotationScale(transform, m_rotation, m_scale);
    }
    
    glm::detail::tvec3<T> m_position;
    glm::detail::tquat<T> m_rotation;
    glm::detail::tvec3<T> m_scale;
};

#endif