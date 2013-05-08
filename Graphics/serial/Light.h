#ifndef ILL_LIGHT_H_
#define ILL_LIGHT_H_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Util/geometry/Plane.h"
#include "Util/serial/Array.h"

namespace illGraphics {

const size_t MAX_LIGHT_VOLUME_PLANES = 12;

struct LightBase {
public:
    enum class Type {
        INVALID,
        POINT,
        POINT_NOSPECULAR,
        SPOT,
        SPOT_NOSPECULAR,
        POINT_VOLUME,
        POINT_VOLUME_NOSPECULAR,
        DIRECTIONAL_VOLUME,
        DIRECTIONAL_VOLUME_NOSPECULAR
    };

    glm::vec3 m_color;
    glm::mediump_float m_intensity;

    inline Type getType() const {
        return m_type;
    }

protected:
    LightBase()
        : m_type(Type::INVALID)
    {}

    LightBase(const glm::vec3& color, glm::mediump_float intensity)
        : m_type(Type::INVALID),
        m_color(color),
        m_intensity(intensity)
    {}

    Type m_type;
};

struct PointLight : public LightBase {
    PointLight()
        : LightBase()
    {
        m_type = Type::POINT;
    }

    PointLight(const glm::vec3& color, glm::mediump_float intensity, bool specular,
        glm::mediump_float attenuationStart, glm::mediump_float attenuationEnd) 
        : LightBase(color, intensity),
        m_attenuationStart(attenuationStart),
        m_attenuationEnd(attenuationEnd)
    {
        m_type = specular ? Type::POINT : Type::POINT_NOSPECULAR;
    }

    glm::mediump_float m_attenuationStart;
    glm::mediump_float m_attenuationEnd;
};

struct SpotLight : public PointLight {
    SpotLight()
        : PointLight()
    {
        m_type = Type::SPOT;
    }

    SpotLight(const glm::vec3& color, glm::mediump_float intensity, bool specular,
        glm::mediump_float attenuationStart, glm::mediump_float attenuationEnd,
        glm::mediump_float coneStart, glm::mediump_float coneEnd)
        : PointLight(color, intensity, false, attenuationStart, attenuationEnd),
        m_coneStart(coneStart),
        m_coneEnd(coneEnd)
    {
        m_type = specular ? Type::SPOT : Type::SPOT_NOSPECULAR;
    }

    glm::mediump_float m_coneStart;
    glm::mediump_float m_coneEnd;
};

struct VolumeLight : public LightBase {
    VolumeLight()
        : LightBase()
    {
        m_type = Type::DIRECTIONAL_VOLUME;
    }

    VolumeLight(const glm::vec3& color, glm::mediump_float intensity, bool specular, bool directional, const glm::vec3 vector)
        : LightBase(color, intensity),
        m_vector(vector)
    {
        m_type = directional 
            ? specular ? Type::DIRECTIONAL_VOLUME : Type::DIRECTIONAL_VOLUME_NOSPECULAR
            : specular ? Type::POINT_VOLUME : Type::POINT_VOLUME_NOSPECULAR;
    }
    
    /**
    If this is a directional light, this is the light direction.
    If this is a point light, this is the origin of the light
    */
    glm::vec3 m_vector;

    /**
    The planes that make up the bounds of the light volume
    */
    Plane<> m_planes[MAX_LIGHT_VOLUME_PLANES];

    /**
    For each plane, this is the reciprocal of the distance from the plane that light
    begins to fade
    */
    glm::mediump_float m_planeFalloff[MAX_LIGHT_VOLUME_PLANES];
};

}

#endif