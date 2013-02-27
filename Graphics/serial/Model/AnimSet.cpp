#include "Logging/logging.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"

#include "Graphics/serial/Model/AnimSet.h"
#include "Util/serial/Array.h"

const uint64_t ANIMSET_MAGIC = 0x494C414E53455430;		//ILANSET0 in big endian 64 bit

namespace illGraphics {

void AnimSet::unload() {
    if(m_state == RES_LOADING) {
        LOG_FATAL_ERROR("Attempting to unload anim set while it's loading");
    }

    if(m_state == RES_UNINITIALIZED || m_state == RES_UNLOADED) {
        return;
    }

    m_boneNameMap.clear();
    
    m_state = RES_UNLOADED;
}

void AnimSet::reload(RendererBackend * rendererBackend) {
    unload();

    m_loader = rendererBackend;

    m_state = RES_LOADING;
	
    illFileSystem::File * openFile = illFileSystem::fileSystem->openRead(m_loadArgs.m_path.c_str());
	
    //read magic string
    {
		uint64_t magic;
        openFile->readB64(magic);

        if(magic != ANIMSET_MAGIC) {
            LOG_FATAL_ERROR("Not a valid ILANSET0 file.");      //TODO: make this not fatal somehow
        }
    }

	//read number of bones
	uint16_t numBones;
	openFile->readL16(numBones);

    //read bone names
	{
		Array<char> strBuffer;

		for(unsigned int bone = 0; bone < numBones; bone++) {
			uint16_t stringBufferLength = openFile->readStringBufferLength();			
			strBuffer.reserve(stringBufferLength);
			openFile->readString(&strBuffer[0], stringBufferLength);

			if(m_boneNameMap.find(&strBuffer[0]) != m_boneNameMap.end()) {
				LOG_ERROR("Animset %s has duplicate bone name %s.  This will cause problems when animating.", m_loadArgs.m_path.c_str(), &strBuffer[0]);
			}
			else {
				m_boneNameMap[&strBuffer[0]] = bone;
			}
		}
	}

	delete openFile;

    m_state = RES_LOADED;
}

}