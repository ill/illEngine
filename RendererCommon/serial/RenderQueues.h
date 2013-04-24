#ifndef ILL_RENDER_QUEUES_H_
#define ILL_RENDER_QUEUES_H_

#include <set>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

#include "Graphics/serial/Light.h"

namespace illGraphics {
class Mesh;
class Material;
class ShaderProgram;
}

namespace illRendererCommon {

class LightNode;
class StaticMeshNode;

//TODO: For now, my maps of maps of maps implementation generates warnings, I'll find a better way later
#pragma warning( disable : 4503 )

/**
*/
struct RenderQueues {
    /**
    Whether or not to get the lights that affect solid objects.
    This should be false if doing deferred shading, but true if doing forward rendering.
    */
    bool m_getSolidAffectingLights;

    /**
    Whether or not to queue lights.
    This should be true if doing deferred shading, but false if doing forward rendering.
    */
    bool m_queueLights;
        
    //TODO: for now using std::maps, I may in the future use something more efficient if needed, maybe radix sort lists of radix sorted lists, etc...

    /**
    Solid static meshes to be drawn in the depth only pass for the currently drawn cell.
    Storing their transforms sorted by mesh.
    */
    struct StaticMeshInfo {
        const StaticMeshNode * m_node;
        uint8_t m_primitiveGroup;
    };

    std::unordered_map<const illGraphics::ShaderProgram *, 
        std::unordered_map<const illGraphics::Material *, 
            std::unordered_map<const illGraphics::Mesh *, std::vector<StaticMeshInfo>>>> m_depthPassSolidStaticMeshes;
    
    struct StaticMeshLightInfo {
        std::set<LightNode *> m_affectingLights;
        StaticMeshInfo m_meshInfo;
    };

    /**
    Solid static meshes to be drawn during the render pass.
    Sorted by shader, material (which does include the shader, but also includes other stuff), then mesh.
    */
    std::unordered_map<const illGraphics::ShaderProgram *, 
        std::unordered_map<const illGraphics::Material *, 
            std::unordered_map<const illGraphics::Mesh *, std::vector<StaticMeshLightInfo>>>>
        m_solidStaticMeshes;

    /**
    Unsolid static meshes to be drawn during the render pass with some blend mode.
    Sorted by shader, material (which does include the shader, but also includes other stuff), then mesh.
    */
    std::unordered_map<const illGraphics::ShaderProgram *, 
        std::unordered_map<const illGraphics::Material *, 
            std::unordered_map<const illGraphics::Mesh *, std::vector<StaticMeshLightInfo>>>> 
        m_unsolidStaticMeshes;

    //TODO: figure out the depth sorted stuff, I probably don't need to sort by state here
    /**
    Unsolid static meshes to be drawn during the render pass with some blend mode that requires depth sorting.
    Sorted by shader, material (which does include the shader, but also includes other stuff), then mesh.
    */
    std::unordered_map<const illGraphics::ShaderProgram *, 
        std::unordered_map<const illGraphics::Material *, 
            std::unordered_map<const illGraphics::Mesh *, std::vector<StaticMeshLightInfo>>>> 
        m_unsolidDepthsortedStaticMeshes;

    /**
    Lights to be drawn in deferred shading sorted by light type and light.
    This should make lights instanceable.
    */
    std::unordered_map<const illGraphics::LightBase::Type, 
        std::unordered_map<illGraphics::LightBase *, std::vector<const LightNode *>>> m_lights;
};

}

#endif