#ifndef ILL_SKELETON_ANIMATION_H__
#define ILL_SKELETON_ANIMATION_H__

#include <map>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Graphics/serial/Model/LastFrameInfo.h"

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
    struct AnimData {
        AnimData()
            : m_numPositionKeys(0),
              m_numRotationKeys(0),
              m_numScalingKeys(0),

              m_positionKeys(NULL),
              m_rotationKeys(NULL),
              m_scalingKeys(NULL)
        {}

        ~AnimData() {
            delete[] m_positionKeys;
            delete[] m_rotationKeys;
            delete[] m_scalingKeys;
        }

        template <typename T>
        struct Key {
            glm::mediump_float m_time;
            T m_data;
        };

        glm::mat4 getTransform(glm::mediump_float time, glm::mediump_float duration, LastFrameInfo& lastFrameInfo = LastFrameInfo()) const;

        unsigned int m_numPositionKeys;
        unsigned int m_numRotationKeys;
        unsigned int m_numScalingKeys;

        Key<glm::vec3> * m_positionKeys;
        Key<glm::quat> * m_rotationKeys;
        Key<glm::vec3> * m_scalingKeys;
    };

    SkeletonAnimation()
        : ResourceBase(),
          m_duration(0.0f)
    {}

    virtual ~SkeletonAnimation() {
        unload();
    }

    virtual void unload();
    virtual void reload(RendererBackend * renderer);

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
     * Gets a bone's transform some time in the animation in seconds.
     * Allows looping of passing in seconds past the duration and negative times and all that.
     */
    inline bool getTransform(const char * boneName, glm::mediump_float time, glm::mat4& destination, LastFrameInfo& lastFrameInfo = LastFrameInfo()) const {
        std::map<std::string, AnimData>::const_iterator iter = m_boneAnimation.find(boneName);

        if(iter == m_boneAnimation.end()) {
            return false;
        }
        else {
            destination = iter->second.getTransform(time, m_duration, lastFrameInfo);

            return true;
        }
    }

    inline const std::map<std::string, AnimData>& getAnimations() const {
        return m_boneAnimation;
    }

private:
    glm::mediump_float m_duration;                      ///<Duration in seconds
    std::map<std::string, AnimData> m_boneAnimation;    ///<map of bone name to animation
};

}

#endif
