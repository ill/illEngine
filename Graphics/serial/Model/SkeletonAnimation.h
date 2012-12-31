#ifndef ILL_SKELETON_ANIMATION_H__
#define ILL_SKELETON_ANIMATION_H__

#include <map>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"
#include "Util/Geometry/Transform.h"

namespace illGraphics {

class RendererBackend;

//TODO: put this together right, at the moment it's just quickly thrown together

struct SkeletonAnimationLoadArgs {
    std::string m_path; //path of skeleton animation file

    //TODO: more to come?  Maybe?
};

class SkeletonAnimation : public ResourceBase<SkeletonAnimation, SkeletonAnimationLoadArgs, RendererBackend> {
public:
    /**
    Information about interpolation between two keyframes for a point in time.
    In most cases frame2 will come directly after frame1 but sometimes frame1 might be the last frame and frame2 may be the first frame as the animation is looping.
    */
    struct InterpInfo {
        unsigned int m_frame1;          ///<The first keyframe being interpolated
        unsigned int m_frame2;          ///<The second keyframe being interpolated
        glm::mediump_float m_delta;     ///<How much interpolation between the two keyframes
    };

    SkeletonAnimation()
        : ResourceBase(),
        m_numFrames(0),
        m_frameRate(0.0f)
    {}

    virtual ~SkeletonAnimation() {
        unload();
    }

    virtual void unload();
    virtual void reload(RendererBackend * renderer);

    inline unsigned int getNumBones() const {
        return (unsigned int) m_boneAnimation.size();
    }

    inline unsigned int getNumFrames() const {
        return m_numFrames;
    }

    inline glm::mediump_float getFrameRate() const {
        return m_frameRate;
    }

    InterpInfo getFrames(glm::mediump_float time) const;

    inline const Transform<> * getTransform(const char * boneName, unsigned int frame) {
        std::map<std::string, Transform<>*>::const_iterator iter = m_boneAnimation.find(boneName);

        if(iter == m_boneAnimation.end()) {
            LOG_ERROR("No bone with name %s in animation", boneName);
            return NULL;
        }
        else {
            return iter->second + frame;
        }
    }

    inline const std::map<std::string, Transform<>*>& getAnimations() const {
        return m_boneAnimation;
    }

private:
    unsigned int m_numFrames;
    glm::mediump_float m_frameRate;
    std::map<std::string, Transform<>*> m_boneAnimation;  //map of bone name to animation
};

}

#endif
