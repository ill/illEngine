#ifndef ILL_CAMERA_H__
#define ILL_CAMERA_H__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "Util/Geometry/Plane.h"
#include "Util/Geometry/Box.h"
#include "Util/Geometry/Frustum.h"
#include "Util/Geometry/geomUtil.h"
#include "Util/util.h"

#include "Graphics/serial/Camera/CameraTransform.h"

namespace Graphics {  

const float DEFAULT_FOV = 90.0f;
const float DEFAULT_ASPECT = 1.0f;
const float DEFAULT_NEAR = 0.1f;
const float DEFAULT_FAR = 1000.0f;

class Camera {
public:
    Camera() {}

    /**
    TODO: document

    @param aspect The aspect ratio of the camera's view.
    When lights compute shadow volumes they usually need an aspect ratio of 1.
    When rendering the view to the screen pass in the aspect ratio stored in the vid_aspectRatio console variable
    @param near The near plane of the camera viewing.  Determines the view frustum in the end.
    @param far The far plane of the camera viewing.  Determines the view frustum in the end.
    */
    inline void setTransform(const CameraTransform& transform, glm::mediump_float aspect = DEFAULT_ASPECT, glm::mediump_float fov = DEFAULT_FOV, glm::mediump_float nearVal = DEFAULT_NEAR, glm::mediump_float farVal = DEFAULT_FAR, bool ortho = false) {
        m_frustum.m_dirty = true;
        m_canonical.m_dirty = true;
        m_modelView.m_dirty = true;
        m_projection.m_dirty = true;
        m_normal.m_dirty = true;

        m_aspect = aspect;
        m_fov = fov;
        m_near = nearVal;
        m_far = farVal;
        m_ortho = ortho;      

        m_transform = transform;
    }

    inline glm::mediump_float getAspect() const {
        return m_aspect;
    }

    inline glm::mediump_float getNear() const {
        return m_near;
    }

    inline glm::mediump_float getFar() const {
        return m_far;
    }

    inline bool getOrtho() const {
        return m_ortho;
    }

    inline const CameraTransform& getTransform() const {
        return m_transform;
    }

    //get the view frustum 
    inline const Frustum<>& getViewFrustum() const {
        if(m_frustum.m_dirty) {
            computeFrustum();
        }

        return m_frustum.m_value;
    }

    //get this for outside transforming
    inline const glm::mat4& getModelView() const {
        if(m_modelView.m_dirty) {
            computeModelView();
        }

        return m_modelView.m_value;
    }

    inline const glm::mat4& getProjection() const {
        if(m_projection.m_dirty) {
            computeProjection();
        }

        return m_projection.m_value;
    }

    inline const glm::mat4& getCanonical() const {
        if(m_canonical.m_dirty) {
            computeCanonical();
        }

        return m_canonical.m_value;
    }

    //only send this to the shader if not using an outside transformed model view matrix
    inline const glm::mat3& getNormal() const {
        if(m_normal.m_dirty) {
            computeNormal();
        }

        return m_normal.m_value;
    }

private:
    //these are marked as const so the getters can be "const"
    inline void computeFrustum() const {
        //multiply modelView * projection to get canonical matrix   
        m_frustum.m_value.set(getCanonical());

        m_frustum.m_dirty = false;
    }

    inline void computeModelView() const {
        m_modelView.m_value = glm::affineInverse(m_transform.m_transform);
        m_modelView.m_dirty = false;
    }

    inline void computeProjection() const {
        if(m_ortho) {
            m_projection.m_value = glm::ortho(-0.5f * m_aspect, 0.5f * m_aspect,
                -0.5f, 0.5f,
                m_near, m_far);
        }
        else {
            m_projection.m_value = glm::perspective(m_fov, m_aspect, m_near, m_far);
        }   

        m_projection.m_dirty = false;
    }

    inline void computeCanonical() const {
        m_canonical.m_value = getProjection() * getModelView();

        m_canonical.m_dirty = false;
    }

    inline void computeNormal() const {
        m_normal.m_value = glm::mat3(getModelView());   
        m_normal.m_dirty = false;
    }

    CameraTransform m_transform;

    //TODO: take out these dirty bit container things, I've found this "optimization" is unnecessary

    //these are marked as mutable so the "const" getters can remain const since they only get computed on read
    mutable DirtyBitContainer<Frustum<> > m_frustum;            ///<view frustum
    mutable bool m_ortho;                                       ///<if the view is in orthographic or perspective projection

    mutable DirtyBitContainer<glm::mat4> m_modelView;           ///<model view matrix
    mutable DirtyBitContainer<glm::mat4> m_projection;          ///<projection matrix in array form for sending down to OpenGL
    mutable DirtyBitContainer<glm::mat4> m_canonical;           ///<canonical matrix (AKA modelViewProjection)
    mutable DirtyBitContainer<glm::mat3> m_normal;              ///<normal matrix in array form for sending down to OpenGL

    //TODO: these should go back into the camera transform so they can be animated
    glm::mediump_float m_aspect;        ///<aspect ratio
    glm::mediump_float m_near;          ///<near plane
    glm::mediump_float m_far;           ///<far plane
    glm::mediump_float m_fov;           ///<field of view
};

}

#endif
