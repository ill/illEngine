#ifndef ILL_LIGHT_H_
#define ILL_LIGHT_H_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace illGraphics {

struct LightBase {
public:
    enum class Type {
        INVALID,
        POINT,
        SPOT,
        DIRECTIONAL
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

    PointLight(const glm::vec3& color, glm::mediump_float intensity, 
        glm::mediump_float attenuationStart, glm::mediump_float attenuationEnd) 
        : LightBase(color, intensity),
        m_attenuationStart(attenuationStart),
        m_attenuationEnd(attenuationEnd)
    {
        m_type = Type::POINT;
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

    SpotLight(const glm::vec3& color, glm::mediump_float intensity,
        glm::mediump_float attenuationStart, glm::mediump_float attenuationEnd,
        glm::mediump_float coneStart, glm::mediump_float coneEnd)
        : PointLight(color, intensity, attenuationStart, attenuationEnd),
        m_coneStart(coneStart),
        m_coneEnd(coneEnd)
    {
        m_type = Type::SPOT;
    }

    glm::mediump_float m_coneStart;
    glm::mediump_float m_coneEnd;
};

struct DirectionLight : public LightBase {
    DirectionLight()
        : LightBase()
    {
        m_type = Type::DIRECTIONAL;
    }

    DirectionLight(const glm::vec3& color, glm::mediump_float intensity, const glm::vec3 direction)
        : LightBase(color, intensity),
        m_direction(direction)
    {
        m_type = Type::DIRECTIONAL;
    }

    glm::vec3 m_direction;
};

}

#endif