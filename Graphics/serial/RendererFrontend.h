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
class MeshNode;

struct RenderArgs {
    const Camera* m_camera;
        
    /*struct MeshInfo {
        std::vector<
    };*/

    //TODO: add lights list sorted by light type and track which lights affect each mesh for forward rendering

    std::unordered_map<Mesh *, std::unordered_map<Material, MeshNode *> > m_solidMeshes;
    std::unordered_map<Mesh *, std::unordered_map<Material, MeshNode *> > m_unsolidMeshes;
    std::unordered_map<Mesh *, std::unordered_map<Material, MeshNode *> > m_unsolidDepthsortedMeshes;
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