#ifndef ILL_RENDERER_FRONTEND_H__
#define ILL_RENDERER_FRONTEND_H__

#include <glm/glm.hpp>

namespace illGraphics {

class RendererBackend;
class BitmapFont;
class Camera;

class RendererFrontend {
private:
    /*struct RenderArgsBase {
        RenderArgsBase()
            : m_camera(NULL),
            m_viewPortId(0),
            m_viewPortOrigin(0),
            m_viewPortDimensions(0)
        {}

        ///the camera angle to render from
        const Camera* m_camera;

        ///the viewport id, for retreiving occlusion query results later
        unsigned int m_viewPortId;

        ///the viewport lower left corner on the screen
        glm::ivec2 m_viewPortOrigin;

        ///the width and height of the viewport
        glm::uvec2 m_viewPortDimensions;

        ///The cull counter so nodes know which frames they were last occluded
        uint64_t m_cullCounter;
    };*/

public:
    /*struct RenderArgs : public RenderArgsBase {
        RenderArgs()
            : RenderArgsBase()
        {}

        inline void addMesh(MeshNode * node) {
            //TODO: later sort by mesh and materials and by nodes that are to be rendered unoclluded, and nodes that are to be rendered to test for occlusion
            //const Node::OcclusionResult& occlusionResult = node->getOcclusionResults(m_viewPortId);

            if(occlusionResult.m_counter < m_cullCounter || occlusionResult.m_pixels == 0) {
                m_occlusionTestNodes.push_back(node);
            }
            else {
                m_staticMeshes.push_back(node);
            }
        }

        //TODO: threadsafe concurrent vector?
        std::vector<MeshNode *> m_meshes;

        ///Nodes that weren't visible last frame so occlusion query them with a simple bounding box
        //std::vector<Node *> m_occlusionTestNodes;
    };

    void render(const RenderArgs& renderArgs);*/

    enum FontHorzAlign {
        FN_H_LEFT, FN_H_CENTER, FN_H_RIGHT
    };

    enum FontVertAlign {
        FN_V_TOP, FN_V_CENTER, FN_V_BOTTOM
    };

    void renderText(const char * text, const glm::mat4& transform, const BitmapFont& font, const Camera& camera, FontHorzAlign horzAlign = FN_H_LEFT, FontVertAlign vertAlign = FN_V_TOP);

private:

    RendererBackend * m_backend;
};

}

#endif