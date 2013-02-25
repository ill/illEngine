#ifndef ILL_MODEL_ANIMATION_CONTROLLER_H__
#define ILL_MODEL_ANIMATION_CONTROLLER_H__

#include <cassert>
#include <unordered_map>
#include <queue>

#include <glm/glm.hpp>
#include "Util/serial/RefCountPtr.h"
#include "Graphics/serial/Model/LastFrameInfo.h"
#include "Graphics/serial/Model/Skeleton.h"

namespace illGraphics {

class SkeletonAnimation;

//TODO: put this together right, at the moment it's just quickly thrown together and doesn't even support blending
//This is in no way how it'll be in the end
class ModelAnimationController {
public:
	ModelAnimationController()
		: m_skeleton(NULL),
		m_skelMats(NULL),
        m_currentAnimation(false),
		m_transitionWeight(0.0f),
		m_transitionDelta(0.0f)
	{}
	    
    ~ModelAnimationController() {
        delete[] m_skelMats;
    }

	inline void init(Skeleton * skeleton) {
		assert(!m_skeleton);

		m_skeleton = skeleton;
		m_skelMats = new glm::mat4[m_skeleton->getNumBones()];
	}

	/**
	Updates the timing and transitions in the animation
	*/
    inline void update(float seconds) {
		float remainSeconds = seconds;

		while(remainSeconds > 0.0f) {
			remainSeconds = updateInternal(remainSeconds);
		}
	}

	/**
	Computes the animation pose given the current internal state of timings.
	*/
    void computeAnimPose();

    /**
    Queues up an animation transition to happen after an already queued up transition.
    This is mostly for convenience given the current setup, and will change completely once animation trees are in.

    @param animation The skeletal animation to start playing
    @param transitionSeconds How many seconds the transition should be
    @param triggerTime After how many seconds in the previous animation should the transition begin.
    @param beginTime At how many seconds should the animation begin.
    */
    inline void queueTransition(SkeletonAnimation * animation, float transitionSeconds, float triggerTime, float beginTime = 0.0f) {
		m_transitionQueue.emplace(animation, transitionSeconds, triggerTime, beginTime);
	}

    /**
    Returns the transformed bones array after computing the pose.
    */
    inline const glm::mat4 * getSkeleton() const {
        return m_skelMats;
    }

    inline size_t getNumBones() const {
        return m_skeleton->getNumBones();
    }

private:
	/**
	The internal update code.

	@return if a transition happened, how much remaining time is left after the transition to possibly perform more transitions
	*/
	float updateInternal(float seconds);

    /**
    Info about an individual animation, these are the things that are blended together
    */
    struct Animation {
        Animation()
            : m_animation(NULL),
            m_animTime(0.0f)
        {}

        ///The skeleton animation that's playing
        SkeletonAnimation * m_animation;

        ///The time in the animation
        float m_animTime;

        ///The map of skeleton bone index to frame info
        std::unordered_map<size_t, LastFrameInfo> m_lastFrameInfo;
	};

    struct Transition {
		Transition(SkeletonAnimation * animation, 
			float transitionSeconds, 
			float triggerTime, 
			float beginTime)
			: m_animation(animation),
			m_transitionSeconds(transitionSeconds),
			m_triggerTime(triggerTime),
			m_beginTime(beginTime)
		{}

        ///The skeleton animation that will play
        SkeletonAnimation * m_animation;

        float m_transitionSeconds;

        float m_triggerTime;

        float m_beginTime;
    };

	void updateSkeleton(const illGraphics::Skeleton::BoneHeirarchy * currNode, glm::mat4 currXform, glm::mat4 currBindXform);

    Skeleton * m_skeleton;    
    
    /**
    Right now there's support for simple blending between two animations.
    Eventually this will support animation trees and all that crazyness.
    */
    Animation m_animations[2];
    bool m_currentAnimation;
    glm::mediump_float m_transitionWeight;
    glm::mediump_float m_transitionDelta;

    glm::mat4 * m_skelMats;                                 //This is the full bone array

    std::queue<Transition> m_transitionQueue;
};

}

#endif
