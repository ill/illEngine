#ifndef __CAMERATRANSFORM_H__
#define __CAMERATRANSFORM_H__

#include <glm/glm.hpp>

namespace Graphics {
   
/**
Camera transforms can be animated and combined in various ways.
After you get the final camera transform pass it in to the Camera with setTranform to take effect before drawing the next frame.
Camera transforms are kept separate from the camera to save memory when you have many transforms that are layered on top of each other.
*/
struct CameraTransform {
    CameraTransform() {}

    /**
    @param transform 4x4 matrix representing rotation and position of the camera.   
    @param fov The field of view.
    Changing this can allow zooming in with a sniper scope or creating some really trippy effects.
    */
    CameraTransform(const glm::mat4& transform)
        : m_transform(transform)
    {}

    /**
    Returns a new camera transform which is this transformed by other.
    Other transform acts as an offset to fov.
    Other transform's actual rotation and position transform transforms this rotation and position transform
    */
    inline CameraTransform getTransformBy(const CameraTransform& other) const {
        return CameraTransform(m_transform * other.m_transform);
    }

    //TODO: rewrite these to use GLM

    /**
    Sets the transform rotation based on a direction
    direction should be a normalized direction vector
    up is the direction the top of the camera is pointing in to allow rolling

    Camera position should be set separately by just doing m_transform.t = <position>
    */
    /*inline void lookAtDirection(const NxVec3& direction, const NxVec3& up) {
    m_transform.M = mat33Inverse(vec3ToMat33(direction, up));            //TODO: make sure this works (why am I doing an inverse?)
    }*/

    /**
    Sets the transform rotation based on a look at a point in space relative to the camera
    up is the direction the top of the camera is pointing in to allow rolling

    Camera position should be set separately by just doing m_transform.t = <position> before calling this
    */
    /*inline void lookAtPoint(const NxVec3& point, const NxVec3& up) {
    lookAtDirection(vec3Normalize(m_transform.t - point), up);

    //m_transform.M = mat33Inverse(vec3ToMat33(vec3Normalize(m_transform.t - point), up));      //TODO: make sure this works (why am I doing an inverse?)
    }*/

    glm::mat4 m_transform;              ///<represents 4x4 matrix storing rotation and position   
};

}

#endif