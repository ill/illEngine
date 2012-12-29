#include <GL/glew.h>
#include "GlCommon/serial/GlRenderer.h"
#include "Util/Geometry/MeshData.h"

namespace GlCommon {

void GlRenderer::loadMesh(void** meshBackendData, const MeshData<>& meshFrontendData) {
    *meshBackendData = new GLuint[2];

    glGenBuffers(2, (GLuint *)(*meshBackendData));
   
    glBindBuffer(GL_ARRAY_BUFFER, *((GLuint *)(*meshBackendData) + 0));
    glBufferData(GL_ARRAY_BUFFER, meshFrontendData.getNumVert() * meshFrontendData.getVertexSize(), meshFrontendData.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *((GLuint *)(*meshBackendData) + 1));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshFrontendData.getNumTri() * 3 * sizeof(uint32_t), meshFrontendData.getIndeces(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GlRenderer::unloadMesh(void** meshBackendData) {
    glDeleteBuffers(2, (GLuint *)(*meshBackendData));
    delete[] (GLuint *)(*meshBackendData);
    *meshBackendData = NULL;
}

}
