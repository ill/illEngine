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
    inline void setPerspectiveTransform(const glm::mat4& transform, glm::mediump_float aspect = DEFAULT_ASPECT, glm::mediump_float fov = DEFAULT_FOV, glm::mediump_float nearVal = DEFAULT_NEAR, glm::mediump_float farVal = DEFAULT_FAR) {
        m_transform = transform;
        m_modelView = glm::affineInverse(m_transform);
        m_projection = glm::perspective(fov, aspect, nearVal, farVal);
        m_canonical = getProjection() * getModelView();
        m_frustum.set(getCanonical());
    }

    inline void setOrthoTransform(const glm::mat4& transform, glm::mediump_float left, glm::mediump_float right, glm::mediump_float bottom, glm::mediump_float top, glm::mediump_float nearVal = 0.0f, glm::mediump_float farVal = 1.0f) {
        m_transform = transform;
        m_modelView = glm::affineInverse(m_transform);
        m_projection = glm::ortho(left, right, bottom, top, nearVal, farVal);
        m_canonical = getProjection() * getModelView();
        m_frustum.set(getCanonical());
    }
    
    inline const glm::mat4& getTransform() const {
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
    glm::mat4 m_transform;              ///<the rotation, position, scale that defines the camera position and orientation
    
    Frustum<> m_frustum;                ///<view frustum

    glm::mat4 m_modelView;              ///<model view matrix
    glm::mat4 m_projection;             ///<projection matrix in array form for sending down to OpenGL
    glm::mat4 m_canonical;              ///<canonical matrix (AKA modelViewProjection)
};

}

#endif
