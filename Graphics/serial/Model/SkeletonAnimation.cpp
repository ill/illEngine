#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "SkeletonAnimation.h"

#include "Logging/logging.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "Util/Geometry/Transform.h"

const uint64_t ANIM_MAGIC = 0x494C4C414E494D30;		//ILLANIM0 in big endian 64 bit

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
    while(lastFrameInfo.m_lastPositionKey < m_positionKeys.size() - 1) {
        if(time < m_positionKeys[lastFrameInfo.m_lastPositionKey + 1].m_time) {
            break;
        }
        lastFrameInfo.m_lastPositionKey++;
    }

    while(lastFrameInfo.m_lastRotationKey < m_rotationKeys.size() - 1) {
        if(time < m_rotationKeys[lastFrameInfo.m_lastRotationKey + 1].m_time) {
            break;
        }
        lastFrameInfo.m_lastRotationKey++;
    }

    while(lastFrameInfo.m_lastScalingKey < m_scalingKeys.size() - 1) {
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
        size_t key1 = lastFrameInfo.m_lastPositionKey;
        size_t key2 = (lastFrameInfo.m_lastPositionKey + 1) % m_positionKeys.size();

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
        size_t key1 = lastFrameInfo.m_lastRotationKey;
        size_t key2 = (lastFrameInfo.m_lastRotationKey + 1) % m_rotationKeys.size();

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
        size_t key1 = lastFrameInfo.m_lastScalingKey;
        size_t key2 = (lastFrameInfo.m_lastScalingKey + 1) % m_scalingKeys.size();

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

void SkeletonAnimation::reload(GraphicsBackend * backend) {
    unload();

    m_loader = backend;

    m_state = RES_LOADING;
	
    illFileSystem::File * openFile = illFileSystem::fileSystem->openRead(m_loadArgs.m_path.c_str());
	
	//read magic string
    {
		uint64_t magic;
        openFile->readB64(magic);

        if(magic != ANIM_MAGIC) {
            LOG_ERROR("Not a valid ILLANIM0 file.");
            m_state = RES_UNLOADED;
            delete openFile;
            return;
        }
    }
	
    //read duration
    openFile->readLF(m_duration);

    //read the number of bones
	uint16_t numBones;
	openFile->readL16(numBones);
    
    //read the bone keyframes
    for(unsigned bone = 0; bone < numBones; bone++) {
        //get the bone index
        uint16_t boneIndex;
		openFile->readL16(boneIndex);
        
        AnimData * currentAnim;

        if(m_boneAnimation.find(boneIndex) != m_boneAnimation.end()) {
            LOG_ERROR("Skeleton animation %s has duplicate bone index %d. This will cause problems when animating. Aborting loading animation.", 
				m_loadArgs.m_path.c_str(), boneIndex);
            m_state = RES_LOADED;
            delete openFile;
            unload();
            return;
        }
        else {
            currentAnim = &m_boneAnimation[boneIndex];
        }

        //read the number of position keys
		{
			uint16_t numElements;
			openFile->readL16(numElements);
			currentAnim->m_positionKeys.resize(numElements);
		}

        //read the position keys themselves
        for(unsigned int frame = 0; frame < currentAnim->m_positionKeys.size(); frame++) {
            //read the timestamp
            openFile->readLF(currentAnim->m_positionKeys[frame].m_time);

            //read the position
            openFile->readLF(currentAnim->m_positionKeys[frame].m_data.x);
            openFile->readLF(currentAnim->m_positionKeys[frame].m_data.y);
            openFile->readLF(currentAnim->m_positionKeys[frame].m_data.z);
        }

        //read the number of rotation keys
		{
			uint16_t numElements;
			openFile->readL16(numElements);
			currentAnim->m_rotationKeys.resize(numElements);
		}

        //read the position keys themselves
        for(unsigned int frame = 0; frame < currentAnim->m_rotationKeys.size(); frame++) {
            //read the timestamp
            openFile->readLF(currentAnim->m_rotationKeys[frame].m_time);

            //read the position
            openFile->readLF(currentAnim->m_rotationKeys[frame].m_data.x);
            openFile->readLF(currentAnim->m_rotationKeys[frame].m_data.y);
            openFile->readLF(currentAnim->m_rotationKeys[frame].m_data.z);
            openFile->readLF(currentAnim->m_rotationKeys[frame].m_data.w);
        }

        //read the number of scaling keys
		{
			uint16_t numElements;
			openFile->readL16(numElements);
			currentAnim->m_scalingKeys.resize(numElements);
		}

        //read the scaling keys themselves
        for(unsigned int frame = 0; frame < currentAnim->m_scalingKeys.size(); frame++) {
            //read the timestamp
            openFile->readLF(currentAnim->m_scalingKeys[frame].m_time);

            //read the position
            openFile->readLF(currentAnim->m_scalingKeys[frame].m_data.x);
            openFile->readLF(currentAnim->m_scalingKeys[frame].m_data.y);
            openFile->readLF(currentAnim->m_scalingKeys[frame].m_data.z);
        }
    }
    
    delete openFile;

    m_state = RES_LOADED;
}

}
