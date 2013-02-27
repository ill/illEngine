#include <map>

#include "Graphics/serial/Model/Skeleton.h"

#include "Logging/logging.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"

const uint64_t SKEL_MAGIC = 0x494C4C534B454C30;		//ILLSKEL0 in big endian 64 bit

namespace illGraphics {

void Skeleton::unload() {
    if(m_state == RES_LOADING) {
        LOG_FATAL_ERROR("Attempting to unload skeleton while it's loading");
    }

    if(m_state == RES_UNINITIALIZED || m_state == RES_UNLOADED) {
        return;
    }
	
    delete m_heirarchy;
    m_heirarchy = NULL;
    
    m_state = RES_UNLOADED;
}

void Skeleton::reload(RendererBackend * rendererBackend) {
    unload();

    m_loader = rendererBackend;

    m_state = RES_LOADING;
	
    illFileSystem::File * openFile = illFileSystem::fileSystem->openRead(m_loadArgs.m_path.c_str());
	
	//read magic string
    {
		uint64_t magic;
        openFile->readB64(magic);

        if(magic != SKEL_MAGIC) {
            LOG_FATAL_ERROR("Not a valid ILLSKEL0 file.");      //TODO: make this not fatal somehow, I guess all meshes would be in their rest pose if they're supposed to have an associated skeleton
        }
    }
	
    //read number of bones
	{
		uint16_t numBones;
		openFile->readL16(numBones);
		m_bones.resize(numBones);
	}

    //read the bind poses
    for(unsigned bone = 0; bone < m_bones.size(); bone++) {
		//bind pose
		for(unsigned int matCol = 0; matCol < 4; matCol++) {
            for(unsigned int matRow = 0; matRow < 4; matRow++) {
				openFile->readLF(m_bones[bone].m_relativeTransform[matCol][matRow]);
            }            
        }

		//offset
		for(unsigned int matCol = 0; matCol < 4; matCol++) {
            for(unsigned int matRow = 0; matRow < 4; matRow++) {
				openFile->readLF(m_bones[bone].m_offsetTransform[matCol][matRow]);
            }            
        }
    }

    //read the heirarchy
    {
        std::map<unsigned int, BoneHeirarchy *> boneToNode;

        for(uint16_t bone = 0; bone < m_bones.size(); bone++) {
            //BoneHeirarchy * currNode = boneToNode[bone];

            //lookup parent node
            uint16_t parentInd;
			openFile->readL16(parentInd);

            //look up the parent node
            BoneHeirarchy * parentNode;
            
			//if the bone is referencing itself, it's the root
            if(parentInd != bone) {
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
		
	delete openFile;

    m_state = RES_LOADED;
}

}
