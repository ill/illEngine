#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "ModelAnimationController.h"
#include "Skeleton.h"
#include "SkeletonAnimation.h"

void updateSkeleton(const illGraphics::Skeleton * skeleton, const illGraphics::Skeleton::BoneHeirarchy * currNode, glm::mat4 currXform, glm::mat4 currBindXform, std::map<unsigned int, glm::mat4>& animTransforms, glm::mat4 * animationTestSkelMats) {
    //currXform = currXform * skeleton->getBone(currNode->m_boneIndex)->m_transform * animTransforms[currNode->m_boneIndex];

    std::map<unsigned int, glm::mat4>::iterator iter = animTransforms.find(currNode->m_boneIndex);

    /*glm::mat4 relXform; //TODO: This should go into the animation file instead of absolute transforms from the bind pose, there's currently a limitation though so it's computed here

    if(iter != animTransforms.end()) {
        relXform = glm::inverse(skeleton->getBone(currNode->m_boneIndex)->m_transform) * animTransforms[currNode->m_boneIndex];
    }

    currXform = currXform * skeleton->getBone(currNode->m_boneIndex)->m_transform * relXform;
    currBindXform = currBindXform * skeleton->getBone(currNode->m_boneIndex)->m_transform;
    currAnimXform = currAnimXform * relXform;*/
    
    if(iter != animTransforms.end()) {
        currXform = currXform * animTransforms[currNode->m_boneIndex];
    }
    else {
        currXform = currXform * skeleton->getBone(currNode->m_boneIndex)->m_transform;
    }

    currBindXform = currBindXform * skeleton->getBone(currNode->m_boneIndex)->m_transform;

    animationTestSkelMats[currNode->m_boneIndex] = 
        //glm::inverse(currBindXform) * currXform;
        currXform * glm::inverse(currBindXform) * glm::rotate(-90.0f, glm::vec3(1.0f, 0.0, 0.0f));      //TODO: for now hardcoded to rotate this -90 degrees around x since all md5s seem to be flipped
                                                                                                        //figure out how to export models in the right orientation
                                                                                                        //THIS!  is why there's the horrible hack code below and why it took me days to get this working, 1 tiny mistake and it all explodes
           
    for(std::vector<illGraphics::Skeleton::BoneHeirarchy *>::const_iterator iter = currNode->m_children.begin(); iter != currNode->m_children.end(); iter++) {
        updateSkeleton(skeleton, *iter, currXform, currBindXform, animTransforms, animationTestSkelMats);
    }
}

namespace illGraphics {

void ModelAnimationController::update(float seconds) {
    illGraphics::SkeletonAnimation::InterpInfo interpInfo = m_animation->getFrames(m_animTime);

    //get transforms for all the bones
    for(std::map<std::string, unsigned int>::const_iterator iter = m_skeleton->getBoneNameMap().begin(); iter != m_skeleton->getBoneNameMap().end(); iter++) {
        std::map<std::string, Transform<>*>::const_iterator animIter = m_animation->getAnimations().find(iter->first);
        
        if(animIter == m_animation->getAnimations().end()) {
            continue;
        }

        Transform<>* transforms = animIter->second;

        Transform<> currentTransform = transforms[interpInfo.m_frame1].interpolate(transforms[interpInfo.m_frame2], interpInfo.m_delta);

        //compute the matrix
        //Oh god!  It took me 2 days of trying to debug math problems to fix this.  It was backwards the whole time.  Damnit Google, this isn't DirectX.
        glm::mat4 transform = currentTransform.getMatrix();
                
        //place the transform into the thing
        m_animationTest[iter->second] = transform;
    }

    m_animTime += seconds;
}

void ModelAnimationController::computeAnimPose() {
    updateSkeleton(m_skeleton, m_skeleton->getRootBoneNode(), glm::mat4(), glm::mat4(), m_animationTest, m_skelMats);
}

}
