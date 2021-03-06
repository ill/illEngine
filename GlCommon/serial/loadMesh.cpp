#include <GL/glew.h>
#include "GlCommon/serial/GlBackend.h"
#include "Util/Geometry/MeshData.h"

namespace GlCommon {

void GlBackend::loadMesh(void** meshBackendData, const MeshData<>& meshFrontendData) {
    *meshBackendData = new GLuint[2];

    glGenBuffers(2, (GLuint *)(*meshBackendData));
   
    glBindBuffer(GL_ARRAY_BUFFER, *((GLuint *)(*meshBackendData) + 0));
    glBufferData(GL_ARRAY_BUFFER, meshFrontendData.getNumVert() * meshFrontendData.getVertexSize(), meshFrontendData.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *((GLuint *)(*meshBackendData) + 1));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshFrontendData.getNumInd() * sizeof(uint16_t), meshFrontendData.getIndices(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GlBackend::unloadMesh(void** meshBackendData) {
    glDeleteBuffers(2, (GLuint *)(*meshBackendData));
    delete[] (GLuint *)(*meshBackendData);
    *meshBackendData = NULL;
}

}
