#ifndef ILL_DEFERRED_SHADING_BACKEND_H_
#define ILL_DEFERRED_SHADING_BACKEND_H_

#include <stdint.h>
#include <unordered_map>

#include "RendererCommon/RendererBackend.h"
#include "RendererCommon/serial/RenderQueues.h"
#include "Util/serial/Array.h"

template <typename Key, typename T, typename Loader> class ResourceManager;

namespace illGraphics {
class Camera;

class ShaderProgram;
struct ShaderProgramLoader;
typedef uint64_t ShaderProgramId;
typedef ResourceManager<ShaderProgramId, ShaderProgram, ShaderProgramLoader> ShaderProgramManager;
}

namespace illDeferredShadingRenderer {

class DeferredShadingBackend : public illRendererCommon::RendererBackend {
public:
    DeferredShadingBackend(illGraphics::GraphicsBackend * backend)
        : RendererBackend(backend),
        m_state(State::UNINITIALIZED),
        m_debugMode(DebugMode::NONE),
        m_debugOcclusion(false),
        m_debugLights(false),
        m_debugBounds(false),
        m_performCull(true)
    {}

    enum class State {
        INITIALIZED,
        UNINITIALIZED
    };

    virtual void initialize(const glm::uvec2 screenResolution, illGraphics::ShaderProgramManager * shaderProgramManager) = 0;
    virtual void uninitialize() = 0;

    inline State getState() const {
        return m_state;
    }

    /**
    Call this before rendering a frame.
    */
    virtual void setupFrame() = 0;

    /**
    Call this before rendering a view.
    */
    virtual void setupViewport(const illGraphics::Camera& camera) = 0;

    /**
    Retreives the cell queries from the last frame for all viewports.
    Writes the results into this pool of viewport id to cells last viewed frames.
    @param lastViewedFrames The lastViewedFrames cells inside the scene that results will be written to.
    @param lastFrameCounter The last frame that happened as tracked by the scene making this call.
        If a cell was visible during the last frame, this will be the value written as the last visible frame.
    */
    virtual void retreiveCellQueries(std::unordered_map<size_t, Array<uint64_t>>& lastViewedFrames, uint64_t lastFrameCounter) = 0;
    
    /**
    Sets up state to be doing cell queries.
    Call this right before starting to do an occlusionQueryCell call either for the first time or
    after having done a depth pass call.  No need to keep calling this over and over between subsequent
    cell queries since the state is already set up.
    */
    virtual void setupCellQuery() = 0;

    /**
    Call this to end cell query state if you're about to do an occlusionQueryCell call.
    */
    virtual void endCellQuery() = 0;

    /**
    Creates an occlusion query for a scene cell and returns a pointer to the data of the occlusion query
    for the CPU side to pass back to the depth pass function later for that cell.

    @param camera The camera angle to render from.
    @param cellCenter The center of the cell box in world coordinates.  This will be used to draw an invisible
        box that will return the query results.
    @param cellSize The size of the box in world space.
    @param cellArrayIndex The array index of the cell in the scene grid.
    @param viewport Which viewport is this currently being rendered for.

    @return A pointer to API specific data holding occlusion query results.
        Pass this into the depthPass call afterwards.
    */
    virtual void * occlusionQueryCell(const illGraphics::Camera& camera, const glm::vec3& cellCenter, const glm::vec3& cellSize,
        unsigned int cellArrayIndex, size_t viewport) = 0;

    /**
    Do a depth pass of objects currently in the depth pass queue in the passed in render queues.
    This should be done per cell after getting an occlusion query for that cell.

    @param renderQueues The render queues object that has all objects to render for the frame.
    @param camera The camera angle to render from.
    @param cellOcclusionQuery The cell occlusion query returned from the occlusionQueryCell() for the cell this is being called for.
        Just pass that in after doing the occlusionQueryCell call.
    */
    virtual void depthPass(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera, void * cellOcclusionQuery) = 0;

    /**
    Now the scene is ready to be deferred shaded.
    */
    virtual void render(illRendererCommon::RenderQueues& renderQueues, const illGraphics::Camera& camera) = 0;
    
    //different debug modes
    //TODO: make a debug version of the renderer separate from the real renderer
    enum class DebugMode {
        NONE,
        
        DEPTH,
        NORMAL,
        DIFFUSE,
        SPECULAR,

        DIFFUSE_ACCUMULATION,
        SPECULAR_ACCUMULATION
    };

    bool m_performCull;
    bool m_debugOcclusion;
    bool m_debugBounds;
    bool m_debugLights;
    const illGraphics::Camera * m_occlusionCamera;
    DebugMode m_debugMode;

protected:
    State m_state;
};

}

#endif