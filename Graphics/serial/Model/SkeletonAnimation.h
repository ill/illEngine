#ifndef ILL_SKELETON_ANIMATION_H__
#define ILL_SKELETON_ANIMATION_H__

#include <unordered_map>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Graphics/serial/Model/LastFrameInfo.h"

#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"
#include "Util/serial/Array.h"

#include "Util/Geometry/Transform.h"

namespace illGraphics {

class GraphicsBackend;

//TODO: put this together right, at the moment it's just quickly thrown together

struct SkeletonAnimationLoadArgs {
    std::string m_path; //path of skeleton animation file

    //TODO: more to come?  Maybe?
};

class SkeletonAnimation : public ResourceBase<SkeletonAnimationLoadArgs, GraphicsBackend> {
public:
    struct AnimData {
        template <typename T>
        struct Key {
            glm::mediump_float m_time;
            T m_data;
        };

        Transform<> getTransform(glm::mediump_float time, glm::mediump_float duration, LastFrameInfo& lastFrameInfo = LastFrameInfo()) const;
		
        Array<Key<glm::vec3>> m_positionKeys;
        Array<Key<glm::quat>> m_rotationKeys;
        Array<Key<glm::vec3>> m_scalingKeys;
    };

	typedef std::unordered_map<uint16_t, AnimData> BoneAnimationMap;

    SkeletonAnimation()
        : ResourceBase(),
          m_duration(0.0f)
    {}

    virtual ~SkeletonAnimation() {
        unload();
    }

    virtual void unload();
    virtual void reload(GraphicsBackend * backend);

    inline unsigned int getNumBones() const {
        return (unsigned int) m_boneAnimation.size();
    }

    /**
     * Gets the duration in seconds.
     */
    inline glm::mediump_float getDuration() const {
        return m_duration;
    }

    /**
    Gets a bone's transform some time in the animation in seconds relative to the bind pose.
    Allows looping of passing in seconds past the duration and negative times and all that.
	Returns an identity transform if passing in a bone index that isn't affected by this animation.
    */
    inline bool getTransform(uint16_t boneIndex, glm::mediump_float time, Transform<>& dest, LastFrameInfo& lastFrameInfo = LastFrameInfo()) const {
        BoneAnimationMap::const_iterator iter = m_boneAnimation.find(boneIndex);

        if(iter == m_boneAnimation.end()) {
            //return Transform<>();
            return false;
        }
        else {
            //return iter->second.getTransform(time, m_duration, lastFrameInfo);
            dest = iter->second.getTransform(time, m_duration, lastFrameInfo);
            return true;
        }
    }
	
private:
    glm::mediump_float m_duration;          ///<Duration in seconds
    BoneAnimationMap m_boneAnimation;		///<map of bone index to animation
};

typedef uint32_t SkeletonAnimationId;
typedef ConfigurableResourceManager<SkeletonAnimationId, SkeletonAnimation, SkeletonAnimationLoadArgs, GraphicsBackend> SkeletonAnimationManager;

}

#endif
