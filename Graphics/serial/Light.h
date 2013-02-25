#ifndef ILL_LIGHT_H_
#define ILL_LIGHT_H_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace illGraphics {

class Light {
private:
    struct LightBase {
        LightBase() {}
        LightBase(const glm::vec3& color, glm::mediump_float intensity)
            : m_color(color),
            m_intensity(intensity)
        {}

        glm::vec3 m_color;
        glm::mediump_float m_intensity;
    };

public:
    enum class Type {
        INVALID,
        POINT,
        SPOT,
        DIRECTIONAL
    };

    struct PointLight : public LightBase {
        PointLight()
            : LightBase()
        {}

        PointLight(const glm::vec3& color, glm::mediump_float intensity, 
            glm::mediump_float attenuationStart, glm::mediump_float attenuationEnd) 
            : LightBase(color, intensity),
            m_attenuationStart(attenuationStart),
            m_attenuationEnd(attenuationEnd)
        {}

        glm::mediump_float m_attenuationStart;
        glm::mediump_float m_attenuationEnd;
    };

    struct SpotLight : public PointLight {
        SpotLight()
            : PointLight()
        {}

        SpotLight(const glm::vec3& color, glm::mediump_float intensity,
            glm::mediump_float attenuationStart, glm::mediump_float attenuationEnd,
            glm::mediump_float coneStart, glm::mediump_float coneEnd, const glm::quat& direction)
            : PointLight(color, intensity, attenuationStart, attenuationEnd),
            m_coneStart(coneStart),
            m_coneEnd(coneEnd),
            m_direction(direction)
        {}

        glm::mediump_float m_coneStart;
        glm::mediump_float m_coneEnd;
      
        glm::quat m_direction;
    };

    struct DirectionLight : public LightBase {
        DirectionLight()
            : LightBase()
        {}

        DirectionLight(const glm::vec3& color, glm::mediump_float intensity, const glm::vec3 direction)
            : LightBase(color, intensity),
            m_direction(direction)
        {}

        glm::vec3 m_direction;
    };

private:
    Type m_type;

    union Data {
        PointLight m_pointLight;
        SpotLight m_spotLight;
        DirectionLight m_directionLight;
    } m_data;
};



}

#endif