#include "SkeletonAnimation.h"

#include "Logging/logging.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"

namespace illGraphics {

SkeletonAnimation::InterpInfo SkeletonAnimation::getFrames(glm::mediump_float time) const {
    InterpInfo res;

    glm::mediump_float baseFrame = time * m_frameRate;

    res.m_frame1 = ((unsigned int) baseFrame);
    res.m_delta = baseFrame - res.m_frame1;

    res.m_frame1 %= m_numFrames;
    res.m_frame2 = (res.m_frame1 + 1) % m_numFrames;

    return res;
}

void SkeletonAnimation::unload() {
    if(m_state == RES_LOADING) {
        LOG_FATAL_ERROR("Attempting to unload skeleton animation while it's loading");
    }

    if(m_state == RES_UNINITIALIZED || m_state == RES_UNLOADED) {
        return;
    }

    m_numFrames = 0;
    m_frameRate = 0.0f;

    for(std::map<std::string, Transform<>*>::iterator iter = m_boneAnimation.begin(); iter != m_boneAnimation.end(); iter++) {
        delete[] iter->second;
    }

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

        if(magicStr.compare("ILLANIM0") != 0) {
            LOG_ERROR("Not a valid ILLANIM0 file.");
            m_state = RES_UNLOADED;
            delete stream;
            return;
        }
    }

    //read framerate
    (*stream) >> m_frameRate;

    //read number of frames
    (*stream) >> m_numFrames;

    //read the number of bones
    unsigned int numBones;
    (*stream) >> numBones;
    
    //read the bone keyframes
    for(unsigned bone = 0; bone < numBones; bone++) {
        //get the bone name
        std::string name;
        (*stream) >> name;

        Transform<> * currentAnim;
        
        if(m_boneAnimation.find(name) != m_boneAnimation.end()) {
            LOG_ERROR("Skeleton animation %s has duplicate bone name %s.  This will cause problems when animating.", m_loadArgs.m_path.c_str(), name.c_str());
            m_state = RES_LOADED;
            delete stream;
            unload();
            return;
        }
        else {
            m_boneAnimation[name] = currentAnim = new Transform<>[m_numFrames];
        }

        //read the keyframes themselves
        for(unsigned int frame = 0; frame < m_numFrames; frame++) {
            //read the position
            (*stream) >> currentAnim[frame].m_position.x;
            (*stream) >> currentAnim[frame].m_position.y;
            (*stream) >> currentAnim[frame].m_position.z;

            //read the rotation
            (*stream) >> currentAnim[frame].m_rotation.x;
            (*stream) >> currentAnim[frame].m_rotation.y;
            (*stream) >> currentAnim[frame].m_rotation.z;
            (*stream) >> currentAnim[frame].m_rotation.w;

            //read the scale
            (*stream) >> currentAnim[frame].m_scale.x;
            (*stream) >> currentAnim[frame].m_scale.y;
            (*stream) >> currentAnim[frame].m_scale.z;
        }
    }
    
    delete stream;

    delete[] fileData;

    m_state = RES_LOADED;
}

}
