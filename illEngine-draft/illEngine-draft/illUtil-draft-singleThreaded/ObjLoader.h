#ifndef __OBJ_LOADER_H__
#define __OBJ_LOADER_H__

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <string>
#include <sstream>

#include "physFsUtil.h"     //TODO: temporary, who cares though, this whole file is temporary
#include "Geometry/MeshData.h"

/**
This is kind of a temporary class while obj files are still used.
I'm only using this temporarily and intend to have either a custom model format, or properly load .fbx files or some other model format.

It's not the most robust loader, but is guranteed to load obj files exported with 3ds max with these export settings:
flip y/z axis (poser like)
faces: triangles

texture coordinates if contains texture coordinate
normals if contains normals

vertex, normals, tex coords optimize flags also work.  It keeps the filesize smaller
*/

//TODO: document some stuff a bit
template <typename T = glm::mediump_float>
struct ObjLoader {
    struct VertexIdentifier {
        VertexIdentifier()
            : m_positionIndex(0),
            m_normalIndex(0),
            m_texCoordIndex(0)
        {}

        struct Comparator {
            bool operator() (const VertexIdentifier& x, const VertexIdentifier& y) const {
                if(x.m_positionIndex < y.m_positionIndex) {
                    return true;
                }
                else if(x.m_positionIndex == y.m_positionIndex) {            
                    if(x.m_normalIndex < y.m_normalIndex) {
                        return true;
                    }
                    else if(x.m_normalIndex == y.m_normalIndex) {            
                        if(x.m_texCoordIndex < y.m_texCoordIndex) {
                            return true;
                        }
                    }
                }

                return false;
            }
        };

        uint32_t m_positionIndex;
        uint32_t m_normalIndex;
        uint32_t m_texCoordIndex;
    };

    ObjLoader(const std::string& fileName, const glm::detail::tvec3<T>& scaleFactor, typename FeaturesMask features)
        : m_features(features),

        m_scaleFactor(scaleFactor),

        m_vertPos(NULL),
        m_vertNorm(NULL),
        m_vertTexCoord(NULL),

        m_numTri(0),
        m_numVert(0)
    {
        if(featureMaskHasPositions(m_features)) {
            m_vertPos = new std::vector<glm::detail::tvec3<T> >();
        }

        if(featureMaskHasNormals(m_features)) {
            m_vertNorm = new std::vector<glm::detail::tvec3<T> >();
        }

        if(featureMaskHasTexCoords(m_features)) {
            m_vertTexCoord = new std::vector<glm::detail::tvec2<T> >();
        }

        //TODO: LOL this is horrible, inefficient, and temporary
        char * objText;

        readFile(fileName, &objText);
        m_openFile = new std::stringstream(std::string(objText), std::ios_base::in);        
        delete[] objText;
    }

    ~ObjLoader() {
        delete m_vertPos;
        delete m_vertNorm;
        delete m_vertTexCoord;

        delete m_openFile;
    }

    /**
    Parses the .obj file.  Guranteed to parse if meeting above defined conditions, otherwise, meh...
    This is temporary and not very robust.
    */
    void parse() {
        //things will crash due to null pointer dereferencing if the right features aren't enabled, so consider that a sanity check
        std::string currentMaterial = "";
        std::string token;   
        while(m_openFile->good()) {
            (*m_openFile) >> token;

            if(token == "v") {   //parse vertex position         
                m_vertPos->push_back(glm::detail::tvec3<T>());

                (*m_openFile) >> m_vertPos->back().x;
                (*m_openFile) >> m_vertPos->back().y;
                (*m_openFile) >> m_vertPos->back().z;

                m_vertPos->back() *= m_scaleFactor;
            }
            else if(token == "vn") {   //parse vertex normal
                m_vertNorm->push_back(glm::detail::tvec3<T>());

                (*m_openFile) >> m_vertNorm->back().x;
                (*m_openFile) >> m_vertNorm->back().y;
                (*m_openFile) >> m_vertNorm->back().z;

                m_vertNorm->back().x = m_vertNorm->back().x * m_scaleFactor.x < 0.0f ? -1.0f : 1.0f;
                m_vertNorm->back().y = m_vertNorm->back().y * m_scaleFactor.y < 0.0f ? -1.0f : 1.0f;
                m_vertNorm->back().z = m_vertNorm->back().z * m_scaleFactor.z < 0.0f ? -1.0f : 1.0f;
            }
            else if(token == "vt") {   //parse texture coordinate
                m_vertTexCoord->push_back(glm::detail::tvec2<T>());

                T eatZero;

                (*m_openFile) >> m_vertTexCoord->back().x;
                (*m_openFile) >> m_vertTexCoord->back().y;
                (*m_openFile) >> eatZero;
            }
            else if(token == "usemtl") {  //parse material name
                (*m_openFile) >> currentMaterial;
            }
            else if(token == "f") {   //parse face
                m_numTri++;

                //obj file has indeces starting at 1 not 0 so account for that later         
                for(int i = 0; i < 3; i++) {
                    VertexIdentifier vertex;

                    //get position:
                    (*m_openFile) >> vertex.m_positionIndex;

                    //get texture coordinate
                    if(m_features & MF_TEX_COORD) {
                        m_openFile->ignore(1);

                        (*m_openFile) >> vertex.m_texCoordIndex;

                        //get normal
                        if(m_features & MF_NORMAL) {
                            m_openFile->ignore(1);
                            (*m_openFile) >> vertex.m_normalIndex;
                        }
                    }
                    else if(m_features & MF_NORMAL) {   //if storing position and normal
                        m_openFile->ignore(2);
                        (*m_openFile) >> vertex.m_normalIndex;
                    }

                    //check if vertex exists
                    std::map<VertexIdentifier, unsigned int, typename VertexIdentifier::Comparator>::iterator iter = m_vertexIndeces.find(vertex);
                    unsigned int currentVertex;

                    if(iter == m_vertexIndeces.end()) {
                        currentVertex = m_vertexIndeces[vertex] = m_numVert++;
                    }
                    else {
                        currentVertex = iter->second;
                    }

                    //append to the index buffer object of the current draw call
                    m_triangleGroupIndeces[currentMaterial].push_back(currentVertex);
                }
            }
            else {   //ignore the line
                m_openFile->ignore(256, '\n');
            }
        }
    }

    /**
    After parsing, puts data into the mesh passed in.
    */
    void buildMesh(MeshData<T>& mesh) const {
        //copy index buffers
        uint32_t currentIndex = 0;
        uint8_t currTriGroup = 0;

        for(std::map<std::string, std::vector<uint32_t>>::const_iterator triGroupIter = m_triangleGroupIndeces.begin(); triGroupIter != m_triangleGroupIndeces.end(); triGroupIter++, currTriGroup++) {
            const std::vector<uint32_t>& indexBuffer = triGroupIter->second;

            memcpy(mesh.getIndeces() + currentIndex, &indexBuffer.at(0), indexBuffer.size() * sizeof(uint32_t));
            currentIndex += (uint32_t) indexBuffer.size();

            MeshData<T>::TriangleGroup& triangleGroup = mesh.getTriangleGroup(currTriGroup);

            triangleGroup.m_indexOffset = currentIndex;
            triangleGroup.m_elementCount = (uint32_t) indexBuffer.size();
        }

        //copy vertex buffer
        for(std::map<VertexIdentifier, uint32_t, typename VertexIdentifier::Comparator>::const_iterator indexIter = m_vertexIndeces.begin(); indexIter != m_vertexIndeces.end(); indexIter++) {
            const VertexIdentifier& vertexIdentifier = indexIter->first;
            uint32_t vertexIndex = indexIter->second;

            if(mesh.hasPositions()) {
                mesh.getPosition(vertexIndex) = m_vertPos->at(vertexIdentifier.m_positionIndex - 1);
            }

            if(mesh.hasNormals()) {
                mesh.getNormal(vertexIndex) = m_vertNorm->at(vertexIdentifier.m_normalIndex - 1);
            }

            if(mesh.hasTexCoords()) {
                mesh.getTexCoord(vertexIndex) = m_vertTexCoord->at(vertexIdentifier.m_texCoordIndex - 1);
            }
        }

        if(mesh.hasTangents() && mesh.hasNormals() && mesh.hasTexCoords()) {
            mesh.buildTangents();
        }
    }

    FeaturesMask m_features;
    std::stringstream * m_openFile;

    glm::detail::tvec3<T> m_scaleFactor;

    std::vector<glm::detail::tvec3<T> > * m_vertPos;
    std::vector<glm::detail::tvec3<T> > * m_vertNorm;
    std::vector<glm::detail::tvec2<T> > * m_vertTexCoord;

    uint32_t m_numTri;
    uint32_t m_numVert;

    std::map<VertexIdentifier, uint32_t, typename VertexIdentifier::Comparator> m_vertexIndeces;
    std::map<std::string, std::vector<uint32_t>> m_triangleGroupIndeces;
};

#endif