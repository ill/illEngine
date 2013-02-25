#ifndef ILL_RENDERER_FRONTEND_H__
#define ILL_RENDERER_FRONTEND_H__

#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

namespace illGraphics {

class RendererBackend;
class Camera;

class Mesh;
class Light;
class Material;

struct RenderArgs {
    const Camera* m_camera;
       
    struct MeshInfo {
        std::vector<Light *> m_affectingLights;
        glm::mat4 m_transform;
        Mesh * m_mesh;
    };

    struct LightInfo {
        glm::mat4 m_transform;
        Light * m_light;
    };
    
    std::unordered_map<Material *, MeshInfo> m_solidMeshes;
    std::unordered_map<Material *, MeshInfo> m_unsolidMeshes;
    std::unordered_map<Material *, MeshInfo> m_unsolidDepthsortedMeshes;


};

class RendererFrontend {
private:

public:    
    void render(const RenderArgs& renderArgs);

private:

    RendererBackend * m_backend;
};

}

#endif