#ifndef __SKELETON_ANIMATION_H__
#define __SKELETON_ANIMATION_H__

#include <map>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../../illUtil-draft-singleThreaded/ResourceBase.h"
#include "../../illUtil-draft-singleThreaded/ResourceManager.h"

namespace Graphics {

class RendererBackend;

struct SkeletonAnimationLoadArgs {
    std::string m_path; //path of skeleton animation file

    //TODO: more to come?  Maybe?
};

class SkeletonAnimation : public ResourceBase<SkeletonAnimation, SkeletonAnimationLoadArgs, RendererBackend> {
public:
    //TODO: move this to geometry util soon
    struct Transform {
        glm::vec3 m_position;
        glm::quat m_rotation;
        glm::vec3 m_scale;
    };

    struct InterpInfo {
        unsigned int m_frame1;
        unsigned int m_frame2;
        glm::mediump_float m_delta;
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

    inline const Transform * getTransform(const std::string& boneName, unsigned int frame) {
        std::map<std::string, Transform*>::const_iterator iter = m_boneAnimation.find(boneName);

        if(iter == m_boneAnimation.end()) {
            LOG_ERROR("No bone with name %s in animation", boneName.c_str());
            return NULL;
        }
        else {
            return iter->second + frame;
        }
    }

    inline const std::map<std::string, Transform*>& getAnimations() const {
        return m_boneAnimation;
    }

private:
    unsigned int m_numFrames;
    glm::mediump_float m_frameRate;
    std::map<std::string, Transform*> m_boneAnimation;  //map of bone name to animation
};

}

#endif