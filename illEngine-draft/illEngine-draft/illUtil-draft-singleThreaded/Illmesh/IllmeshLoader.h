#ifndef __ILLMESHLOADER_H__
#define __ILLMESHLOADER_H__

#include <string>
#include "../Logging/logging.h"
#include "../Geometry/MeshData.h"
#include "../physFsUtil.h"     //TODO: temporary

/**
Loads the amazing illmesh ASCII format.  It's a nice simple format where you don't deal with any BS and just load what you need into the VBO and IBO.
*/
template <typename T = glm::mediump_float>
struct IllmeshLoader {
    IllmeshLoader(const std::string& fileName)
        : m_features(0),
        m_numVert(0),
        m_numInd(0)
    {
        //TODO: LOL this is horrible, inefficient, and temporary, and works just fine in most situations so whatever...
        //I need to write a nice file IO interface around physfs for PC and whatever we need on Android and Ios when the time comes, for now this will work
        char * objText;

        readFile(fileName, &objText);
        m_openFile = new std::stringstream(std::string(objText), std::ios_base::in);        
        delete[] objText;

        //read header
        {
            std::string magicStr;

            (*m_openFile) >> magicStr;

            if(magicStr.compare("ILLMESH0") != 0) {
                LOG_FATAL_ERROR("Not a valid ILLMESH0 file.");      //TODO: make this not fatal and instead load a crappy little box to indicate that the mesh failed to load
            }
        }

        //read mesh features
        {
            bool feature;

            //has positions?
            (*m_openFile) >> feature;
            if(feature) {
                m_features |= MF_POSITION;
            }

            //has normals?
            (*m_openFile) >> feature;
            if(feature) {
                m_features |= MF_NORMAL;
            }

            //has tangents and bitangents?
            (*m_openFile) >> feature;
            if(feature) {
                m_features |= MF_TANGENT;
            }

            //has tex coords
            (*m_openFile) >> feature;
            if(feature) {
                m_features |= MF_TEX_COORD;
            }

            //has blend data
            (*m_openFile) >> feature;
            if(feature) {
                m_features |= MF_BLEND_DATA;
            }

            //has colors
            (*m_openFile) >> feature;
            if(feature) {
                m_features |= MF_COLOR;
            }
        }

        //read the buffer sizes
        (*m_openFile) >> m_numVert;
        (*m_openFile) >> m_numInd;
    }

    ~IllmeshLoader() {
        delete m_openFile;
    }

    void buildMesh(MeshData<T>& mesh) const {
        //read the VBO, some 1337 hax going on here
        size_t numElements = mesh.getVertexSize() / sizeof(T);

        for(unsigned int vertex = 0, vboIndex = 0; vertex < m_numVert; vertex++) {
            for(unsigned int elementIndex = 0; elementIndex < numElements; elementIndex++, vboIndex += sizeof(T)) {
                (*m_openFile) >> *reinterpret_cast<T *>(mesh.getData() + vboIndex);
            }
        }

        //read the IBO
        for(unsigned int index = 0, iboIndex = 0; index < m_numInd; index++, iboIndex++) {
            (*m_openFile) >> *(mesh.getIndeces() + iboIndex);
        }
    }

    FeaturesMask m_features;
    std::stringstream * m_openFile;

    unsigned int m_numVert;
    unsigned int m_numInd;
};

#endif
