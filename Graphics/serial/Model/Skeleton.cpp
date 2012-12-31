#include <map>

#include "Graphics/serial/Model/Skeleton.h"

#include "Logging/logging.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"

namespace illGraphics {

void Skeleton::unload() {
    if(m_state == RES_LOADING) {
        LOG_FATAL_ERROR("Attempting to unload skeleton while it's loading");
    }

    if(m_state == RES_UNINITIALIZED || m_state == RES_UNLOADED) {
        return;
    }

    m_numBones = 0;
    delete[] m_bones;
    m_bones = NULL;

    m_boneNameMap.clear();

    delete m_heirarchy;
    m_heirarchy = NULL;
    
    m_state = RES_UNLOADED;
}

void Skeleton::reload(RendererBackend * rendererBackend) {
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

        if(magicStr.compare("ILLSKEL0") != 0) {
            LOG_FATAL_ERROR("Not a valid ILLSKEL0 file.");      //TODO: make this not fatal somehow, I guess all meshes would be in their rest pose if they're supposed to have an associated skeleton
        }
    }

    //read number of bones
    (*stream) >> m_numBones;
    m_bones = new Bone[m_numBones];

    //read the bind poses
    for(unsigned bone = 0; bone < m_numBones; bone++) {
        //TODO: hack
        //m_bones[bone].m_boneOffsetHack = NULL;

        for(unsigned int matRow = 0; matRow < 4; matRow++) {
            for(unsigned int matCol = 0; matCol < 4; matCol++) {
                (*stream) >> m_bones[bone].m_transform[matCol][matRow];
            }            
        }
    }

    //read the heirarchy
    {
        std::map<unsigned int, BoneHeirarchy *> boneToNode;

        for(unsigned int bone = 0; bone < m_numBones; bone++) {
            //BoneHeirarchy * currNode = boneToNode[bone];

            //lookup parent node
            int parentInd;
            (*stream) >> parentInd;

            //look up the parent node
            BoneHeirarchy * parentNode;
            
            if(parentInd >= 0 && parentInd != (int) bone) {
                std::map<unsigned int, BoneHeirarchy *>::iterator iter = boneToNode.find(parentInd);

                //if not found, create the new node
                if(iter == boneToNode.end()) {
                    boneToNode[parentInd] = parentNode = new BoneHeirarchy();
                    parentNode->m_boneIndex = parentInd;
                }
                else {
                    parentNode = iter->second;
                }
            }
            else {
                //this is the root node
                parentNode = NULL;

                //if already found a root, error
                if(m_heirarchy) {
                    //this is nonfatal so it's just a warning, the last bone in the list will be considered the root in this case
                    LOG_ERROR("Skeleton %s has multiple root bones.  This case is undefined.", m_loadArgs.m_path.c_str());
                }
            }

            //look up the current node
            BoneHeirarchy * currentNode;

            {
                std::map<unsigned int, BoneHeirarchy *>::iterator iter = boneToNode.find(bone);

                //if not found, create the new node
                if(iter == boneToNode.end()) {
                    boneToNode[bone] = currentNode = new BoneHeirarchy();
                    currentNode->m_boneIndex = bone;
                }
                else {
                    currentNode = iter->second;
                }
            }

            currentNode->m_parent = parentNode;
            
            if(parentNode == NULL) {
                //assign the root
                m_heirarchy = currentNode;
            }
            else {
                parentNode->m_children.push_back(currentNode);
            }
        }
    }

    //read bone names
    {
        for(unsigned int bone = 0; bone < m_numBones; bone++) {
            std::string name;

            (*stream) >> name;

            if(m_boneNameMap.find(name) != m_boneNameMap.end()) {
                LOG_ERROR("Skeleton %s has duplicate bone name %s.  This will cause problems when animating.", m_loadArgs.m_path.c_str(), name.c_str());
            }
            else {
                m_boneNameMap[name] = bone;
            }
        }
    }

    delete stream;    
    delete[] fileData;

    m_state = RES_LOADED;
}

}
