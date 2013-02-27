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
        m_currentAnimation(false),
		m_transitionWeight(0.0f),
		m_transitionDelta(0.0f)
	{}

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
	Computes the animation pose given the current internal state of timings,
	and writes the matrices to the specified buffer.

	The buffer must be allocated to hold as many 4x4 matrices as there are bones in the skeleton.
	This is the data you usually want to pass to the skinning shader or for computing
	hitboxes or whatever...
	*/
    void computeAnimPose(glm::mat4 * skelMats);

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
	
    inline size_t getNumBones() const {
        return m_skeleton->getNumBones();
    }

	/**
	 * Gets the time into the current animation.
	 */
	inline float getAnimationTime() const {
	    return m_animations[m_currentAnimation].m_animTime;
	}

	/**
	 * Gets the time into the animation being transitioned into.
	 */
	inline float getTransitionAnimationTime() const {
        return m_animations[!m_currentAnimation].m_animTime;
    }

	/**
	Sets the skeleton for the animation controller.
	You should pretty much only call this at the beginning before using
	the controller, but it should also be safe to change the skeleton
	if no animations are currently playing by queueing up a NULL animation.

	You can force a NULL animation to queue up immediately by queueing a
	transition with a NULL animation and zero transition time and it should
	now be safe to set the skeleton again.
	*/
	inline void setSkeleton(Skeleton * skeleton) {
		m_skeleton = skeleton;
	}

	inline const Skeleton * getSkeleton() const {
		return m_skeleton;
	}

//TODO: temporarily public
//private:
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

	void updateSkeleton(glm::mat4 * skelMats, const illGraphics::Skeleton::BoneHeirarchy * currNode, glm::mat4 currXform);

    Skeleton * m_skeleton;    
    
    /**
    Right now there's support for simple blending between two animations.
    Eventually this will support animation trees and all that crazyness.
    */
    Animation m_animations[2];
    bool m_currentAnimation;
    glm::mediump_float m_transitionWeight;
    glm::mediump_float m_transitionDelta;

    std::queue<Transition> m_transitionQueue;
};

}

#endif
