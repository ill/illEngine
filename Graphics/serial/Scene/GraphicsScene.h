#ifndef ILL_GRAPHICS_SCENE_H_
#define ILL_GRAPHICS_SCENE_H_

#include <unordered_set>

#include "illEngine/Util/Geometry/GridVolume3D.h"

template <typename T = glm::mediump_float>
struct Frustum;

namespace illGraphics {

class MeshNode;

class GraphicsScene {
private:
   /**
   The contents of a 3D scene grid cell.
   Things that are static should never leave a cell.
   This is useful for keeping the dynamically moving objects in a separate data structure
   and the static objects in their own simpler read only list that won't be interfered with all the time.
   */
   struct SceneCell {
      SceneCell() {}
      ~SceneCell() {}
      
      /**
      The list of meshes.
      */
      std::unordered_set<MeshNode *> m_meshes;

      /**
      The list of static meshes.
      */
      //TODO: make this an array
      std::vector<MeshNode *> m_staticMeshes;
   };

public:
   /**
   Creates the scene and its 3D uniform grid.
   
   @param cellDimensions The dimensions of the grid cells in world units.
   @param cellNumber The number of cells in each dimension.
   */
   GraphicsScene(const glm::vec3& cellDimensions, const glm::uvec3& cellNumber);

   ~GraphicsScene() {}

   /**
   Before rendering a frame, for every viewport you will render call viewLoad().
   After doing that for every viewport call viewUnload() once.

   This will start streaming in the set of resources that will be needed for this frame for all viewports and
   unload the set of resources that won't be needed.

   @param radius The radius of the view.  This doesn't get things in the view frustum but all around the view in case
      the player turns around quickly and everything looks all blurry and ugly for a bit.
   */
   void viewLoad(const Sphere<>& radius);

   /**
   @see viewLoad
   */
   void viewUnload();

   /**
   As you are about to render a viewport, call this to retreive what object are visible in that viewport.
   This does view frustum and occlusion culling on the scene from a point of view.

   You can even reuse the objects returned in this call in a totally different view port to debug draw the effectiveness of the occlusion culling
   and see only what's visible from another camera angle.

   @param viewFrustum The view frustum computed from a camera when you set the camera's transform.
   @param renderArgsDest This is the structure you will pass in to the Renderer::render method.
      It contains the nodes in the scene and will be rendered.
   */
   void viewCull(const Frustum<>& viewFrustum, Renderer::RenderArgs& renderArgsDest);
   
   /**
   Call this after the scene is done loading.
   */
   void doneLoading();

   /**
   Returns the grid volume that is used to manage the scene.
   */
   const GridVolume3D<SceneCell>& getGridVolume() const {
      return m_scene;
   }

private:

   /**
   Called by the node adding itself.
   */
   void addStaticMesh(MeshNode * mesh);

   void addMesh(MeshNode * mesh);
   void removeMesh(MeshNode * mesh);
   void moveMesh(MeshNode * mesh, const std::vector<unsigned int>& toAddCells, const std::vector<unsigned int>& toRemoveCells);
   
   /**
   The frame counter that keeps track of which nodes have already been told to load in the current frame.
   This is to handle nodes that overlap multiple cells in the grid and keeps them from being processed more than once.
   */
   uint64_t m_loadCounter;

   /**
   The counter that keeps track of which nodes have been culled during a call to viewCull.
   This is to handle nodes that overlap multiple cells in the grid and keeps them from being processed more than once.
   */
   uint64_t m_cullCounter;
   
   /**
   The 3D uniform grid for the scene.
   */
   GridVolume3D<> m_grid;

   friend MeshNode;
};

}

#endif