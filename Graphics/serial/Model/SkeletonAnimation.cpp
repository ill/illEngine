#include <glm/gtx/transform.hpp>

#include "SkeletonAnimation.h"

#include "Logging/logging.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"

namespace illGraphics {

Transform<> SkeletonAnimation::AnimData::getTransform(glm::mediump_float time, glm::mediump_float duration, LastFrameInfo& lastFrameInfo) const {
    time = fmod(time, duration);
    
    if(time < lastFrameInfo.m_lastTime) {
        lastFrameInfo.m_lastPositionKey = 0;
        lastFrameInfo.m_lastRotationKey = 0;
        lastFrameInfo.m_lastScalingKey = 0;
    }

    lastFrameInfo.m_lastTime = time;

    //find the right keys
    while(lastFrameInfo.m_lastPositionKey < m_numPositionKeys - 1) {
        if(time < m_positionKeys[lastFrameInfo.m_lastPositionKey + 1].m_time) {
            break;
        }
        lastFrameInfo.m_lastPositionKey++;
    }

    while(lastFrameInfo.m_lastRotationKey < m_numRotationKeys - 1) {
        if(time < m_rotationKeys[lastFrameInfo.m_lastRotationKey + 1].m_time) {
            break;
        }
        lastFrameInfo.m_lastRotationKey++;
    }

    while(lastFrameInfo.m_lastScalingKey < m_numScalingKeys - 1) {
        if(time < m_scalingKeys[lastFrameInfo.m_lastScalingKey + 1].m_time) {
            break;
        }
        lastFrameInfo.m_lastScalingKey++;
    }
    
    //LOG_DEBUG("Time: %f Pos: %u Rot: %u Scale: %u", time, lastKey[0], lastKey[1], lastKey[2]);

    //interpolate between keys
    Transform<> res;

    //position
    {
        unsigned int key1 = lastFrameInfo.m_lastPositionKey;
        unsigned int key2 = (lastFrameInfo.m_lastPositionKey + 1) % m_numPositionKeys;

        glm::mediump_float interp = m_positionKeys[key2].m_time - m_positionKeys[key1].m_time;

        if(interp < 0.0f) {
            interp += duration;
        }

        if(interp == 0.0f) {
            res.m_position = m_positionKeys[key1].m_data;
        }
        else {
            interp = (time - m_positionKeys[key1].m_time) / interp;
            res.m_position = m_positionKeys[key1].m_data + (m_positionKeys[key2].m_data - m_positionKeys[key1].m_data) * interp;
        }
    }

    //rotation
    {
        unsigned int key1 = lastFrameInfo.m_lastRotationKey;
        unsigned int key2 = (lastFrameInfo.m_lastRotationKey + 1) % m_numRotationKeys;

        glm::mediump_float interp = m_rotationKeys[key2].m_time - m_rotationKeys[key1].m_time;

        if(interp < 0.0f) {
            interp += duration;
        }

        if(interp == 0.0f) {
            res.m_rotation = m_rotationKeys[key1].m_data;
        }
        else {
            interp = (time - m_rotationKeys[key1].m_time) / interp;
            res.m_rotation = glm::shortMix(m_rotationKeys[key1].m_data, m_rotationKeys[key2].m_data, interp);
        }
    }

    //scaling
    {
        unsigned int key1 = lastFrameInfo.m_lastScalingKey;
        unsigned int key2 = (lastFrameInfo.m_lastScalingKey + 1) % m_numScalingKeys;

        glm::mediump_float interp = m_scalingKeys[key2].m_time - m_scalingKeys[key1].m_time;

        if(interp < 0.0f) {
            interp += duration;
        }

        if(interp == 0.0f) {
            res.m_scale = m_scalingKeys[key1].m_data;
        }
        else {
            interp = (time - m_scalingKeys[key1].m_time) / interp;
            res.m_scale = m_scalingKeys[key1].m_data + (m_scalingKeys[key2].m_data - m_scalingKeys[key1].m_data) * interp;
        }
    }

    return res;
}

void SkeletonAnimation::unload() {
    if(m_state == RES_LOADING) {
        LOG_FATAL_ERROR("Attempting to unload skeleton animation while it's loading");
    }

    if(m_state == RES_UNINITIALIZED || m_state == RES_UNLOADED) {
        return;
    }

    m_duration = 0;

    m_boneAnimation.clear();
    
    m_state = RES_UNLOADED;
}

void SkeletonAnimation::reload(RendererBackend * rendererBackend) {
    unload();

    m_loader = rendererBackend;

    m_state = RES_LOADING;

    //TODO: LOL this is horrible, inefficient, and temporary, and works just fine in most situations so whatever...
    //I need to write a nice file IO interface around physfs for PC and whatever we need on Android and Ios when the time comes, for now this will work
    char * fileData;

    illFileSystem::File * openFile = illFileSystem::fileSystem->openRead(m_loadArgs.m_path.c_str());

    fileData = new char[openFile->getSize() + 1];
    fileData[openFile->getSize()] = '\0';

    openFile->read(fileData, openFile->getSize());
    std::string streamData(fileData);
    std::stringstream * stream = new std::stringstream(streamData, std::ios_base::in);    

    delete openFile;

    //read header
    {
        std::string magicStr;

        (*stream) >> magicStr;

        if(magicStr.compare("ILLANIM1") != 0) {
            LOG_ERROR("Not a valid ILLANIM1 file.");
            m_state = RES_UNLOADED;
            delete stream;
            return;
        }
    }

    //read duration
    (*stream) >> m_duration;

    //read the number of bones
    unsigned int numBones;
    (*stream) >> numBones;
    
    //read the bone keyframes
    for(unsigned bone = 0; bone < numBones; bone++) {
        //get the bone name
        std::string name;
        (*stream) >> name;
        
        AnimData * currentAnim;

        if(m_boneAnimation.find(name) != m_boneAnimation.end()) {
            LOG_ERROR("Skeleton animation %s has duplicate bone name %s.  This will cause problems when animating.", m_loadArgs.m_path.c_str(), name.c_str());
            m_state = RES_LOADED;
            delete stream;
            unload();
            return;
        }
        else {
            currentAnim = &m_boneAnimation[name];
        }

        //read the number of position keys
        (*stream) >> currentAnim->m_numPositionKeys;
        currentAnim->m_positionKeys = new AnimData::Key<glm::vec3>[currentAnim->m_numPositionKeys];

        //read the position keys themselves
        for(unsigned int frame = 0; frame < currentAnim->m_numPositionKeys; frame++) {
            //read the timestamp
            (*stream) >> currentAnim->m_positionKeys[frame].m_time;

            //read the position
            (*stream) >> currentAnim->m_positionKeys[frame].m_data.x;
            (*stream) >> currentAnim->m_positionKeys[frame].m_data.y;
            (*stream) >> currentAnim->m_positionKeys[frame].m_data.z;
        }

        //read the number of rotation keys
        (*stream) >> currentAnim->m_numRotationKeys;
        currentAnim->m_rotationKeys = new AnimData::Key<glm::quat>[currentAnim->m_numRotationKeys];

        //read the position keys themselves
        for(unsigned int frame = 0; frame < currentAnim->m_numRotationKeys; frame++) {
            //read the timestamp
            (*stream) >> currentAnim->m_rotationKeys[frame].m_time;

            //read the position
            (*stream) >> currentAnim->m_rotationKeys[frame].m_data.x;
            (*stream) >> currentAnim->m_rotationKeys[frame].m_data.y;
            (*stream) >> currentAnim->m_rotationKeys[frame].m_data.z;
            (*stream) >> currentAnim->m_rotationKeys[frame].m_data.w;
        }

        //read the number of scaling keys
        (*stream) >> currentAnim->m_numScalingKeys;
        currentAnim->m_scalingKeys = new AnimData::Key<glm::vec3>[currentAnim->m_numScalingKeys];

        //read the scaling keys themselves
        for(unsigned int frame = 0; frame < currentAnim->m_numScalingKeys; frame++) {
            //read the timestamp
            (*stream) >> currentAnim->m_scalingKeys[frame].m_time;

            //read the position
            (*stream) >> currentAnim->m_scalingKeys[frame].m_data.x;
            (*stream) >> currentAnim->m_scalingKeys[frame].m_data.y;
            (*stream) >> currentAnim->m_scalingKeys[frame].m_data.z;
        }
    }
    
    delete stream;

    delete[] fileData;

    m_state = RES_LOADED;
}

}
