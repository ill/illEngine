#include "SkeletonAnimation.h"

#include "../../illUtil-draft-singleThreaded/Logging/logging.h"
#include "../../illUtil-draft-singleThreaded/physFsUtil.h"

namespace Graphics {

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

    for(std::map<std::string, Transform*>::iterator iter = m_boneAnimation.begin(); iter != m_boneAnimation.end(); iter++) {
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
    char * objText;

    readFile(m_loadArgs.m_path, &objText);
    std::stringstream * openFile = new std::stringstream(std::string(objText), std::ios_base::in);        
    delete[] objText;

    //read header
    {
        std::string magicStr;

        (*openFile) >> magicStr;

        if(magicStr.compare("ILLANIM0") != 0) {
            LOG_ERROR("Not a valid ILLANIM0 file.");
            m_state = RES_UNLOADED;
            delete openFile;
            return;
        }
    }

    //read framerate
    (*openFile) >> m_frameRate;

    //read number of frames
    (*openFile) >> m_numFrames;

    //read the number of bones
    unsigned int numBones;
    (*openFile) >> numBones;
    
    //read the bone keyframes
    for(unsigned bone = 0; bone < numBones; bone++) {
        //get the bone name
        std::string name;
        (*openFile) >> name;

        Transform * currentAnim;
        
        if(m_boneAnimation.find(name) != m_boneAnimation.end()) {
            LOG_ERROR("Skeleton animation %s has duplicate bone name %s.  This will cause problems when animating.", m_loadArgs.m_path.c_str(), name.c_str());
            m_state = RES_LOADED;
            delete openFile;
            unload();
            return;
        }
        else {
            m_boneAnimation[name] = currentAnim = new Transform[m_numFrames];
        }

        //read the keyframes themselves
        for(unsigned int frame = 0; frame < m_numFrames; frame++) {
            //read the position
            (*openFile) >> currentAnim[frame].m_position.x;
            (*openFile) >> currentAnim[frame].m_position.y;
            (*openFile) >> currentAnim[frame].m_position.z;

            //read the rotation
            (*openFile) >> currentAnim[frame].m_rotation.x;
            (*openFile) >> currentAnim[frame].m_rotation.y;
            (*openFile) >> currentAnim[frame].m_rotation.z;
            (*openFile) >> currentAnim[frame].m_rotation.w;

            //read the scale
            (*openFile) >> currentAnim[frame].m_scale.x;
            (*openFile) >> currentAnim[frame].m_scale.y;
            (*openFile) >> currentAnim[frame].m_scale.z;
        }
    }
    
    delete openFile;

    m_state = RES_LOADED;
}

}