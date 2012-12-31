#ifndef ILL_ILLMESHLOADER_H__
#define ILL_ILLMESHLOADER_H__

#include <string>
#include "Logging/logging.h"
#include "Util/Geometry/MeshData.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"

/**
Loads the amazing illmesh ASCII format.  It's a nice simple format where you don't deal with any BS and just load what you need into the VBO and IBO.
*/
template <typename T = glm::mediump_float>
struct IllmeshLoader {
    IllmeshLoader(const char * fileName)
        : m_features(0)
    {
        //TODO: LOL this is horrible, inefficient, and temporary, and works just fine in most situations so whatever...
        //I need to write a nice file IO interface around physfs for PC and whatever we need on Android and Ios when the time comes, for now this will work
        char * fileData;

        illFileSystem::File * openFile = illFileSystem::fileSystem->openRead(fileName);

        fileData = new char[openFile->getSize() + 1];
        fileData[openFile->getSize()] = '\0';

        openFile->read(fileData, openFile->getSize());
        std::string streamData(fileData);
        m_stream = new std::stringstream(streamData, std::ios_base::in);  

        delete openFile;

        //read header
        {
            std::string magicStr;

            (*m_stream) >> magicStr;

            if(magicStr.compare("ILLMESH0") != 0) {
                LOG_FATAL_ERROR("Not a valid ILLMESH0 file.");      //TODO: make this not fatal and instead load a crappy little box to indicate that the mesh failed to load
            }
        }

        //read mesh features
        {
            bool feature;

            //has positions?
            (*m_stream) >> feature;
            if(feature) {
                m_features |= MF_POSITION;
            }

            //has normals?
            (*m_stream) >> feature;
            if(feature) {
                m_features |= MF_NORMAL;
            }

            //has tangents and bitangents?
            (*m_stream) >> feature;
            if(feature) {
                m_features |= MF_TANGENT;
            }

            //has tex coords
            (*m_stream) >> feature;
            if(feature) {
                m_features |= MF_TEX_COORD;
            }

            //has blend data
            (*m_stream) >> feature;
            if(feature) {
                m_features |= MF_BLEND_DATA;
            }

            //has colors
            (*m_stream) >> feature;
            if(feature) {
                m_features |= MF_COLOR;
            }
        }

        //read the buffer sizes
        (*m_stream) >> m_numVert;
        (*m_stream) >> m_numInd;

        delete[] fileData;
    }

    ~IllmeshLoader() {
        delete m_stream;
    }

    void buildMesh(MeshData<T>& mesh) const {
        //read the VBO, some 1337 hax going on here
        size_t numElements = mesh.getVertexSize() / sizeof(T);

        for(unsigned int vertex = 0, vboIndex = 0; vertex < m_numVert; vertex++) {
            for(unsigned int elementIndex = 0; elementIndex < numElements; elementIndex++, vboIndex += sizeof(T)) {
                (*m_stream) >> *reinterpret_cast<T *>(mesh.getData() + vboIndex);
            }
        }

        //read the IBO
        for(unsigned int index = 0, iboIndex = 0; index < m_numInd; index++, iboIndex++) {
            (*m_stream) >> *(mesh.getIndeces() + iboIndex);
        }
    }

    FeaturesMask m_features;
    std::stringstream * m_stream;

    unsigned int m_numVert;
    unsigned int m_numInd;
};

#endif
