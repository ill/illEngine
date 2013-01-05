#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "ModelAnimationController.h"
#include "Skeleton.h"
#include "SkeletonAnimation.h"

void updateSkeleton(const illGraphics::Skeleton * skeleton, const illGraphics::Skeleton::BoneHeirarchy * currNode, glm::mat4 currXform, glm::mat4 currBindXform, std::map<unsigned int, illGraphics::ModelAnimationController::BoneInfo>& animTransforms, glm::mat4 * animationTestSkelMats) {
    std::map<unsigned int, illGraphics::ModelAnimationController::BoneInfo>::iterator iter = animTransforms.find(currNode->m_boneIndex);
        
    if(iter != animTransforms.end()) {
        currXform = currXform * iter->second.m_transform;
    }
    else {
        currXform = currXform * skeleton->getBone(currNode->m_boneIndex)->m_transform;
    }

    currBindXform = currBindXform * skeleton->getBone(currNode->m_boneIndex)->m_transform;

    animationTestSkelMats[currNode->m_boneIndex] = currXform * glm::inverse(currBindXform) 
        * glm::rotate(-90.0f, glm::vec3(1.0f, 0.0, 0.0f));      //TODO: for now hardcoded to rotate this -90 degrees around x since all md5s seem to be flipped
                                                                //figure out how to export models in the right orientation
                                                                //THIS!  is why there's the horrible hack code below and why it took me days to get this working, 1 tiny mistake and it all explodes
           
    for(std::vector<illGraphics::Skeleton::BoneHeirarchy *>::const_iterator iter = currNode->m_children.begin(); iter != currNode->m_children.end(); iter++) {
        updateSkeleton(skeleton, *iter, currXform, currBindXform, animTransforms, animationTestSkelMats);
    }
}

namespace illGraphics {

void ModelAnimationController::update(float seconds) {
    //get transforms for all the bones
    for(std::map<std::string, unsigned int>::const_iterator iter = m_skeleton->getBoneNameMap().begin(); iter != m_skeleton->getBoneNameMap().end(); iter++) {
        glm::mat4 transform;
        
        if(m_animation->getTransform(iter->first.c_str(), m_animTime, transform, m_animationTest[iter->second].m_lastFrameInfo)) {
            //place the transform into the thing
            m_animationTest[iter->second].m_transform = transform;
        }
    }

    m_animTime += seconds;
}

void ModelAnimationController::computeAnimPose() {
    updateSkeleton(m_skeleton, m_skeleton->getRootBoneNode(), glm::mat4(), glm::mat4(), m_animationTest, m_skelMats);
}

}
