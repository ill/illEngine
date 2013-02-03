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
    Sets up the camera for a perspective transform.

    @param transform The position and orientation of the camera.
    @param aspect The aspect ratio of the camera's view, which is usually width / height of the viewport.
        When lights compute shadow volumes they usually need an aspect ratio of 1.
    @param fov The vertical field of view.
    @param nearVal The near plane of the camera viewing.  This needs to be greater than zero.  Determines the view frustum in the end.
    @param farVal The far plane of the camera viewing.  Determines the view frustum in the end.
    */
    inline void setPerspectiveTransform(const glm::mat4& transform, glm::mediump_float aspect = DEFAULT_ASPECT, glm::mediump_float fov = DEFAULT_FOV, glm::mediump_float nearVal = DEFAULT_NEAR, glm::mediump_float farVal = DEFAULT_FAR) {
        m_transform = transform;
        m_modelView = glm::affineInverse(m_transform);
        m_projection = glm::perspective(fov, aspect, nearVal, farVal);
        m_modelViewProjection = getProjection() * getModelView();
        m_frustum.set(getModelViewProjection());
    }

    /**
    Sets up the camera for an orthographic transform.

    @param transform The position and orientation of the camera.
    @param left The left plane
    @param right The right plane
    @param bottom The bottom plane
    @param top The top plane
    @param nearVal The near plane
    @param farVal The far plane
    */
    inline void setOrthoTransform(const glm::mat4& transform, glm::mediump_float left, glm::mediump_float right, glm::mediump_float bottom, glm::mediump_float top, glm::mediump_float nearVal = 0.0f, glm::mediump_float farVal = 1.0f) {
        m_transform = transform;
        m_modelView = glm::affineInverse(m_transform);
        m_projection = glm::ortho(left, right, bottom, top, nearVal, farVal);
        m_modelViewProjection = getProjection() * getModelView();
        m_frustum.set(getModelViewProjection());
    }

    /**
    Sets the viewport on screen.

    @param viewportCorner The position in screen coords of the corner on screen
    @param viewportDimensions The width and height of the screen relative to the corner.  Can be negative if the corner is in a different spot.
    */
    inline void setViewport(const glm::ivec2& viewportCorner, const glm::ivec2& viewportDimensions) {
        m_viewportCorner = viewportCorner;
        m_viewportDimensions = viewportDimensions;
    }

    /**
    Gets the position in screen coords of the viewport corner on screen
    */
    inline const glm::ivec2& getViewportCorner() const {
        return m_viewportCorner;
    }

    /**
    Gets the width and height of the screen relative to the corner.
    */
    inline const glm::ivec2& getViewportDimensions() const {
        return m_viewportDimensions;
    }
    
    /**
    Gets the current position and orientation of the camera.
    */
    inline const glm::mat4& getTransform() const {
        return m_transform;
    }

    /**
    Gets the view frutum, which can be used for culling
    */
    inline const Frustum<>& getViewFrustum() const {
        return m_frustum;
    }

    /**
    Gets the model view matrix, for sending down to shaders.
    This is just the inverse of the transform.
    */
    inline const glm::mat4& getModelView() const {
        return m_modelView;
    }

    /**
    Gets the projection matrix, for sending down to shaders. 
    This affects the view frustum based on perspective or ortho transform.
    */
    inline const glm::mat4& getProjection() const {
        return m_projection;
    }

    /**
    Gets the projection matrix, for sending down to shaders. 
    This is just modelView x projection.
    */
    inline const glm::mat4& getModelViewProjection() const {
        return m_modelViewProjection;
    }

    /**
    Returns a line segment that can be used for 3D picking of objects based on 2D window coordinates.
    This is computed using the camera's current transforms.
    You can use the segment to create a ray if needed.

    @param windowCoords The 2D window coordinates
    @param ptADestination Destination where point A of the line segment will go
    @param ptBDestination Destination where point B of the line segment will go
    */
    inline void getPickSegment(const glm::vec2& windowCoords, glm::vec3& ptADestination, glm::vec3& ptBDestination) const {
        ptADestination = glm::unProject(glm::vec3(windowCoords, 0.0f), m_modelView, m_projection, glm::ivec4(m_viewportCorner, m_viewportDimensions));
        ptBDestination = glm::unProject(glm::vec3(windowCoords, 1.0f), m_modelView, m_projection, glm::ivec4(m_viewportCorner, m_viewportDimensions));
    }
    
private:
    glm::mat4 m_transform;              ///<the rotation, position, scale that defines the camera position and orientation
    
    Frustum<> m_frustum;                ///<view frustum

    glm::mat4 m_modelView;              ///<model view matrix
    glm::mat4 m_projection;             ///<projection matrix
    glm::mat4 m_modelViewProjection;    ///<model view projection matrix

    glm::ivec2 m_viewportCorner;        ///<the viewport corner
    glm::ivec2 m_viewportDimensions;    ///<the viewport width and height relative to the corner
};

}

#endif
