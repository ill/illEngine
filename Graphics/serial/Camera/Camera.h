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

namespace illGraphics {  

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
        m_aspect = aspect;
        m_fov = fov;
        m_near = nearVal;
        m_far = farVal;
        m_ortho = ortho;      

        m_transform = transform;

        m_modelView = glm::affineInverse(m_transform.m_transform);

        if(m_ortho) {
            m_projection = glm::ortho(-0.5f * m_aspect, 0.5f * m_aspect,
                -0.5f, 0.5f,
                m_near, m_far);
        }
        else {
            m_projection = glm::perspective(m_fov, m_aspect, m_near, m_far);
        }

        m_canonical = getProjection() * getModelView();

        m_frustum.set(getCanonical());
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
        return m_frustum;
    }

    //get this for outside transforming
    inline const glm::mat4& getModelView() const {
        return m_modelView;
    }

    inline const glm::mat4& getProjection() const {
        return m_projection;
    }

    inline const glm::mat4& getCanonical() const {
        return m_canonical;
    }

    /**
    Returns a line segment that can be used for 3D picking of objects based on 2D window coordinates.
    You can use the segment to create a ray if needed.

    @param windowCoords The 2D window coordinates
    @param viewportCorner Usually just (0,0) unless you modified the viewport
    @param viewportDimensions Usually the dimensions of the window unless you modified the viewport
    @param ptADestination Destination where point A of the line segment will go
    @param ptBDestination Destination where point B of the line segment will go
    */
    inline void getPickSegment(const glm::vec2& windowCoords, const glm::ivec2& viewportCorner, const glm::ivec2& viewportDimensions,
            glm::vec3& ptADestination, glm::vec3& ptBDestination) const {
        ptADestination = glm::unProject(glm::vec3(windowCoords, 0.0f), m_modelView, m_projection, glm::ivec4(viewportCorner, viewportDimensions));
        ptBDestination = glm::unProject(glm::vec3(windowCoords, 1.0f), m_modelView, m_projection, glm::ivec4(viewportCorner, viewportDimensions));
    }
    
private:

    CameraTransform m_transform;
    
    Frustum<> m_frustum;                ///<view frustum
    bool m_ortho;                       ///<if the view is in orthographic or perspective projection

    glm::mat4 m_modelView;              ///<model view matrix
    glm::mat4 m_projection;             ///<projection matrix in array form for sending down to OpenGL
    glm::mat4 m_canonical;              ///<canonical matrix (AKA modelViewProjection)

    //TODO: these should go back into the camera transform so they can be animated
    glm::mediump_float m_aspect;        ///<aspect ratio
    glm::mediump_float m_near;          ///<near plane
    glm::mediump_float m_far;           ///<far plane
    glm::mediump_float m_fov;           ///<field of view
};

}

#endif
