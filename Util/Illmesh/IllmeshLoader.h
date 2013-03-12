#ifndef ILL_ILLMESHLOADER_H__
#define ILL_ILLMESHLOADER_H__

#include <string>
#include "Logging/logging.h"
#include "Util/Geometry/MeshData.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"

const uint64_t MESH_MAGIC = 0x494C4C4D45534830;	//ILLMESH0 in big endian 64 bit

/**
Loads the amazing illmesh format.  It's a nice simple format where you don't deal with any BS and just load what you need into the VBO and IBO.
*/
struct IllmeshLoader {
    IllmeshLoader(const char * fileName)
        : m_features(0)
    {
        m_openFile = illFileSystem::fileSystem->openRead(fileName);
		
		//read magic string
		{
			uint64_t magic;
			m_openFile->readB64(magic);

			if(magic != MESH_MAGIC) {
				LOG_FATAL_ERROR("Not a valid ILLMESH0 file.");      //TODO: make this not fatal and instead load a crappy little box to indicate that the mesh failed to load
			}
		}
		
        //read mesh features
		m_openFile->read8(m_features);

        //read the buffer sizes
		m_openFile->readL32(m_numVert);
		m_openFile->readL16(m_numInd);
    }

    ~IllmeshLoader() {
        delete m_openFile;
    }

    void buildMesh(MeshData<>& mesh) const {
        //read the VBO, some 1337 hax going on here
        size_t numElements = mesh.getVertexSize() / sizeof(float);

        for(unsigned int vertex = 0, vboIndex = 0; vertex < m_numVert; vertex++) {
            for(unsigned int elementIndex = 0; elementIndex < numElements; elementIndex++, vboIndex += sizeof(float)) {
				m_openFile->readLF(*reinterpret_cast<float *>(mesh.getData() + vboIndex));
            }
        }

        //read the IBO
        for(unsigned int index = 0, iboIndex = 0; index < m_numInd; index++, iboIndex++) {
            m_openFile->readL16(*(mesh.getIndices() + iboIndex));
        }
    }

    FeaturesMask m_features;
    illFileSystem::File * m_openFile;

    uint32_t m_numVert;
    uint16_t m_numInd;
};

#endif
