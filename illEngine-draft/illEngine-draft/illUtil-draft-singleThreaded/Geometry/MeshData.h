#ifndef __MESH_DATA_H__
#define __MESH_DATA_H__

#include <cstdint>
#include <cassert>
#include <glm/glm.hpp>

/**
The vertex features used by this mesh.  Positions will most likely always be present, but everything else is optional.
*/
typedef uint8_t FeaturesMask;

/**
For the bitmask storing which features the mesh has
*/
enum MeshFeatures {
    MF_POSITION = 1 << 0,
    MF_NORMAL = 1 << 1,
    MF_TANGENT = 1 << 2,
    MF_TEX_COORD = 1 << 3,
    MF_BLEND_DATA = 1 << 4,    
    MF_COLOR = 1 << 5
};

/**
Whether or not this features mask has positions.
*/
inline bool featureMaskHasPositions(FeaturesMask features) {
    return (features & MF_POSITION) != 0;
}

/**
Whether or not this features mask has normals.
*/
inline bool featureMaskHasNormals(FeaturesMask features) {
    return (features & MF_NORMAL) != 0;
}

/**
Whether or not this features mask has tangents.
*/
inline bool featureMaskHasTangents(FeaturesMask features) {
    return (features & MF_TANGENT) != 0;
}

/**
Whether or not this features mask has texture coordinates.
*/
inline bool featureMaskHasTexCoords(FeaturesMask features) {
    return (features & MF_TEX_COORD) != 0;
}

/**
Whether or not this features mask has tangents.
*/
inline bool featureMaskHasBlendData(FeaturesMask features) {
    return (features & MF_BLEND_DATA) != 0;
}

/**
Whether or not this features mask has vertex colors.
*/
inline bool featureMaskHasColors(FeaturesMask features) {
    return (features & MF_COLOR) != 0;
}

/**
Contains data about a mesh and methods to access the data.
The vertex data, such as positions, normals, etc... is stored interleaved for efficiency.
The mesh doesn't necessarily have to have its data allocated.
This structure is still useful to keep around to store metadata about the mesh.
It may be useful to free the mesh after creating a vertex buffer object and uploading it to the card, 
and then using the offset methods when drawing the vertex buffer object.

TODO: It'd be nice to rewrite this to be a baseclass using variadic templates and call it InterleavedData,
then have a MeshData class that contains this.

@tparam T The precision of the data inside.  By default it's float.
*/
template <typename T = glm::mediump_float>
class MeshData {
public:
    /**
    Vertex positions
    */
    typedef glm::detail::tvec3<T> Position;

    /**
    Vertex normals
    */
    typedef glm::detail::tvec3<T> Normal;

    /**
    Vertex tangents useful for normal mapping.
    */
    struct TangentData {
        glm::detail::tvec3<T> m_tangent;
        glm::detail::tvec3<T> m_bitangent;
    };

    /**
    The 2d s,t texture coordinates.
    */
    typedef glm::detail::tvec2<T> TexCoord;

    /**
    The skeletal animation blend data.
    */
    struct BlendData {
        glm::detail::tvec4<T> m_blendIndex;
        glm::detail::tvec4<T> m_blendWeight;
    };
    
    /**
    The rgba vertex color.
    */
    typedef glm::detail::tvec4<T> Color;

    /**
    A mesh may need multiple triangle groups, for example if it's a multimaterial mesh and needs separate draw calls per material
    */
    //struct TriangleGroup {
    //    /**
    //    Offset into the vertex index buffer object that starts this triangle group.
    //    This should be the starting triangle multiplied by 3 since triangles are defined in groups of 3 vertices.
    //    */
    //    unsigned int m_indexOffset;

    //    /**
    //    How many vertex indeces this triangle group consists of.
    //    This should be the number of triangles multiplied by 3 since triangles are defined in groups of 3 vertices.
    //    */
    //    unsigned int m_elementCount;
    //};

    /**
    Creates the mesh.
    @param numTri The number of triangles in the mesh.
    @param numVert The number of vertices in the mesh.  Not necessairly numTri multiplied by 3 since there can be shared vertices.
    @param numGroups The number of triangle groups.  This would correspond to the number of materials in a multimaterial mesh for example.
    @param features Which features are stored in the verteces.
    By default it creates a mesh that stores positions, normals, tangents, and texture coordinates.
    @param allocate Whether or not the data for the mesh should be allocated on the CPU side.
    */
    MeshData(uint32_t numTri, uint32_t numVert, /*uint8_t numGroups,*/ FeaturesMask features = MF_POSITION | MF_NORMAL | MF_TANGENT | MF_TEX_COORD, bool allocate = true)
        : m_numVert(numVert),
        m_data(NULL),
        m_numTri(numTri),
        m_indeces(NULL),
        //m_numGroups(numGroups),
        m_features(features)
    {
        free();

        m_positionOffset = 0;

        m_normalOffset = m_positionOffset;
        if(hasPositions()) {
            m_normalOffset += sizeof(Position);
        }

        m_tangentOffset = m_normalOffset;
        m_bitangentOffset = m_tangentOffset + sizeof(glm::detail::tvec3<T>);
        if(hasNormals()) {
            m_tangentOffset += sizeof(Normal);
            m_bitangentOffset += sizeof(Normal);
        }

        m_blendIndexOffset = m_tangentOffset;
        m_blendWeightOffset = m_blendIndexOffset + sizeof(glm::detail::tvec4<T>);
        if(hasTangents()) {
            m_blendIndexOffset += sizeof(TangentData);
            m_blendWeightOffset += sizeof(TangentData);
        }

        m_texCoordOffset = m_blendIndexOffset;
        if(hasBlendData()) {
            m_texCoordOffset += sizeof(BlendData);
        }

        m_colorOffset = m_texCoordOffset;
        if(hasTexCoords()) {
            m_colorOffset += sizeof(TexCoord);
        }

        m_vertexSize = m_colorOffset;
        if(hasColors()) {
            m_vertexSize += sizeof(Color);
        }

        //m_triangleGroups = new TriangleGroup[m_numGroups];

        if(allocate) {
            this->allocate();
        }
    }

    ~MeshData() {
        free();

        //delete[] m_triangleGroups;
    }

    /**
    Allocates memory for the mesh on the CPU side.
    */
    inline void allocate() {
        free();

        m_data = new uint8_t[m_numVert * m_vertexSize];
        m_indeces = new uint32_t[m_numTri * 3];      
    }

    /**
    Frees memory for the mesh from the CPU side.
    Call this after uploading the data into a vertex buffer object on the GPU or something.
    All methods that don't return references to the data will still return valid values.
    */
    inline void free() {
        delete[] m_data;
        delete[] m_indeces;
    }

    /**
    Whether or not this mesh has positions.
    */
    inline bool hasPositions() const {
        return featureMaskHasPositions(m_features);
    }

    /**
    Whether or not this mesh has normals.
    */
    inline bool hasNormals() const {
        return featureMaskHasNormals(m_features);
    }

    /**
    Whether or not this mesh has tangents.
    */
    inline bool hasTangents() const {
        return featureMaskHasTangents(m_features);
    }

    /**
    Whether or not this mesh has blend data.
    */
    inline bool hasBlendData() const {
        return featureMaskHasBlendData(m_features);
    }

    /**
    Whether or not this mesh has texture coordinates.
    */
    inline bool hasTexCoords() const {
        return featureMaskHasTexCoords(m_features);
    }

    /**
    Whether or not this mesh has vertex colors.
    */
    inline bool hasColors() const {
        return featureMaskHasColors(m_features);
    }

    /**
    Gets an offset into the data where positions start.
    Invalid if positions aren't stored.
    */
    inline size_t getPositionOffset() const {
        assert(hasPositions());

        return m_positionOffset;
    }

    /**
    Gets an offset into the data where normals start.
    Invalid if normals aren't stored.
    */
    inline size_t getNormalOffset() const {
        assert(hasNormals());

        return m_normalOffset;
    }

    /**
    Gets an offset into the data where tangents start.
    Invalid if tangents aren't stored.
    */
    inline size_t getTangentOffset() const {
        assert(hasTangents());

        return m_tangentOffset;
    }

    /**
    Gets an offset into the data where bitangents start.
    Invalid if tangents aren't stored.
    */
    inline size_t getBitangentOffset() const {
        assert(hasTangents());

        return m_bitangentOffset;
    }

    /**
    Gets an offset into the data where blend indices start.
    Invalid if blend data isn't stored.
    */
    inline size_t getBlendIndexOffset() const {
        assert(hasBlendData());

        return m_blendIndexOffset;
    }

    /**
    Gets an offset into the data where blend weights start.
    Invalid if blend data isn't stored.
    */
    inline size_t getBlendWeightOffset() const {
        assert(hasBlendData());

        return m_blendWeightOffset;
    }

    /**
    Gets an offset into the data where texture coordinates start.
    Invalid if texture coordinates aren't stored.
    */
    inline size_t getTexCoordOffset() const {
        assert(hasTexCoords());

        return m_texCoordOffset;
    }

    /**
    Gets an offset into the data where vertex colors start.
    Invalid if colors aren't stored.
    */
    inline size_t getColorOffset() const {
        assert(hasColors());

        return m_colorOffset;
    }

    /**
    Returns the size of vertices.
    This is affected by what features are stored.
    Use this as a parameter for any functions that take stride of data between vertices.
    */
    inline size_t getVertexSize() const {
        return m_vertexSize;
    }

    /**
    Returns the size of faces.
    This is just the sice you get from getVertexSize() multiplied by 3.
    Use this as a parameter for any functions that take stride of data between face.
    */
    inline size_t getFaceSize() const {
        return m_vertexSize * 3;
    }

    /**
    Get a reference to a vertex position by triangle face.
    You can use this reference to modify the internal data inside too.
    This is invalid if the data has been freed.

    @param faceInd The triangle face index.
    @param vertInd The face vertex index, a value from 0 to 2.
    */
    inline Position& getPosition(uint32_t faceInd, uint32_t vertInd) {
        assert(hasPositions());
        assert(faceInd < m_numTri);
        assert(vertInd < 3);
        assert(m_data);

        return reinterpret_cast<Position&>(*(m_data + m_positionOffset + *(m_indeces + faceInd * 3 + vertInd) * m_vertexSize));
    }

    /**
    Get a reference to a vertex position.
    You can use this reference to modify the internal data inside too.
    This is invalid if the data has been freed.

    @param vertInd The face vertex index in the vertex array.
    */
    inline Position& getPosition(uint32_t vertInd) {
        assert(hasPositions());
        assert(vertInd < m_numVert);
        assert(m_data);

        return reinterpret_cast<Position&>(*(m_data + m_positionOffset + vertInd * m_vertexSize));
    }

    /**
    Get a reference to a vertex normal.
    You can use this reference to modify the internal data inside too.
    This is invalid if the data has been freed.

    @param faceInd The triangle face index.
    @param vertInd The face vertex index, a value from 0 to 2.
    */
    inline Normal& getNormal(uint32_t faceInd, uint32_t vertInd) {
        assert(hasNormals());
        assert(faceInd < m_numTri);
        assert(vertInd < 3);
        assert(m_data);

        return reinterpret_cast<Normal&>(*(m_data + m_normalOffset + *(m_indeces + faceInd * 3 + vertInd) * m_vertexSize));
    }

    /**
    Get a reference to a vertex normal.
    You can use this reference to modify the internal data inside too.
    This is invalid if the data has been freed.

    @param vertInd The face vertex index in the vertex array.
    */
    inline Normal& getNormal(uint32_t vertInd) {
        assert(hasNormals());
        assert(vertInd < m_numVert);
        assert(m_data);

        return reinterpret_cast<Normal&>(*(m_data + m_normalOffset + vertInd * m_vertexSize));
    }

    /**
    Get a reference to a vertex tangent.
    You can use this reference to modify the internal data inside too.
    This is invalid if the data has been freed.

    @param faceInd The triangle face index.
    @param vertInd The face vertex index, a value from 0 to 2.
    */
    inline TangentData& getTangent(uint32_t faceInd, uint32_t vertInd) {
        assert(hasTangents());
        assert(faceInd < m_numTri);
        assert(vertInd < 3);
        assert(m_data);

        return reinterpret_cast<TangentData&>(*(m_data + m_tangentOffset + *(m_indeces + faceInd * 3 + vertInd) * m_vertexSize));
    }

    /**
    Get a reference to a vertex tangent.
    You can use this reference to modify the internal data inside too.
    This is invalid if the data has been freed.

    @param vertInd The face vertex index in the vertex array.
    */
    inline TangentData& getTangent(uint32_t vertInd) {
        assert(hasTangents());
        assert(vertInd < m_numVert);
        assert(m_data);

        return reinterpret_cast<TangentData&>(*(m_data + m_tangentOffset + vertInd * m_vertexSize));
    }

    /**
    Get a reference to a vertex blend data.
    You can use this reference to modify the internal data inside too.
    This is invalid if the data has been freed.

    @param faceInd The triangle face index.
    @param vertInd The face vertex index, a value from 0 to 2.
    */
    inline BlendData& getBlendData(uint32_t faceInd, uint32_t vertInd) {
        assert(hasBlendData());
        assert(faceInd < m_numTri);
        assert(vertInd < 3);
        assert(m_data);

        return reinterpret_cast<BlendData&>(*(m_data + m_blendIndexOffset + *(m_indeces + faceInd * 3 + vertInd) * m_vertexSize));
    }

    /**
    Get a reference to a vertex blend index.
    You can use this reference to modify the internal data inside too.
    This is invalid if the data has been freed.

    @param vertInd The face vertex index in the vertex array.
    */
    inline BlendData& getBlendData(uint32_t vertInd) {
        assert(hasBlendData());
        assert(vertInd < m_numVert);
        assert(m_data);

        return reinterpret_cast<BlendData&>(*(m_data + m_blendIndexOffset + vertInd * m_vertexSize));
    }

    /**
    Get a reference to a vertex texture coordinate.
    You can use this reference to modify the internal data inside too.
    This is invalid if the data has been freed.

    @param faceInd The triangle face index.
    @param vertInd The face vertex index, a value from 0 to 2.
    */
    inline TexCoord& getTexCoord(uint32_t faceInd, uint32_t vertInd) {
        assert(hasTexCoords());
        assert(faceInd < m_numTri);
        assert(vertInd < 3);
        assert(m_data);

        return reinterpret_cast<TexCoord&>(*(m_data + m_texCoordOffset + *(m_indeces + faceInd * 3 + vertInd) * m_vertexSize));
    }

    /**
    Get a reference to a texture coordinate.
    You can use this reference to modify the internal data inside too.
    This is invalid if the data has been freed.

    @param vertInd The face vertex index in the vertex array.
    */
    inline TexCoord& getTexCoord(uint32_t vertInd) {
        assert(hasTexCoords());
        assert(vertInd < m_numVert);
        assert(m_data);

        return reinterpret_cast<TexCoord&>(*(m_data + m_texCoordOffset + vertInd * m_vertexSize));
    }

    /**
    Get a reference to a vertex color.
    You can use this reference to modify the internal data inside too.
    This is invalid if the data has been freed.

    @param faceInd The triangle face index.
    @param vertInd The face vertex index, a value from 0 to 2.
    */
    inline Color& getColor(uint32_t faceInd, uint32_t vertInd) {
        assert(hasColors());
        assert(faceInd < m_numTri);
        assert(vertInd < 3);
        assert(m_data);

        return reinterpret_cast<Color&>(*(m_data + m_colorOffset + *(m_indeces + faceInd * 3 + vertInd) * m_vertexSize));
    }

    /**
    Get a reference to a vertex color.
    You can use this reference to modify the internal data inside too.
    This is invalid if the data has been freed.

    @param vertInd The face vertex index in the vertex array.
    */
    inline Color& getColor(uint32_t vertInd) {
        assert(hasColors());
        assert(vertInd < m_numVert);
        assert(m_data);

        return reinterpret_cast<Color&>(*(m_data + m_colorOffset + vertInd * m_vertexSize));
    }

    /**
    Computes the tangents for the mesh based on the normals and texture coordinates.
    Invalid if normals, texture coordinates, and tangents aren't stored, or if the data isn't allocated.
    */
    void buildTangents() {
        assert(hasTangents());
        assert(hasNormals());
        assert(hasTexCoords());
        assert(m_data);

        //TODO: rewrite this to not use handedness and instead compute the bitangent, this isn't as useful now that I have Assimp (HAHAHHAHAHAHA Assimp)

        /*for(unsigned int triangleIndex = 0; triangleIndex < m_numTri; triangleIndex++) {
            // Gram-Schmidt orthogonalize tangents and handedness
            glm::detail::tvec3<T> posVec01 = getPosition(triangleIndex, 1) - getPosition(triangleIndex, 0);
            glm::detail::tvec3<T> posVec02 = getPosition(triangleIndex, 2) - getPosition(triangleIndex, 0);

            glm::detail::tvec3<T> texVec01 = glm::detail::tvec3<T>(getTexCoord(triangleIndex, 1) - getTexCoord(triangleIndex, 0), 0.0f);
            glm::detail::tvec3<T> texVec02 = glm::detail::tvec3<T>(getTexCoord(triangleIndex, 2) - getTexCoord(triangleIndex, 0), 0.0f);

            T denom = 1 / (texVec01.x * texVec02.y - texVec02.x * texVec01.y);

            glm::detail::tvec3<T> sDir = (posVec01 * texVec02.y - posVec02 * texVec01.y) * denom;
            glm::detail::tvec3<T> tDir = (posVec02 * texVec01.x - posVec01 * texVec02.x) * denom;

            for(unsigned int vertIndex = 0; vertIndex < 3; vertIndex++) {
                glm::detail::tvec3<T> norm = glm::detail::tvec3<T>(getNormal(triangleIndex, vertIndex));
                glm::detail::tvec3<T> orthoTangent = (sDir - norm * glm::dot(norm, sDir));
                orthoTangent = glm::normalize(orthoTangent);

                //tangent
                Tangent& tangent = getTangent(triangleIndex, vertIndex);

                tangent.x = orthoTangent.x;
                tangent.y = orthoTangent.y;
                tangent.z = orthoTangent.z;

                //handedness
                tangent.w = glm::dot(tDir, glm::cross(norm, sDir)) < (T)0 ? (T)-1 : (T)1;
            }
        }*/
    }

    //TODO: add a build normals function later if useful...

    /**
    Get a pointer to the data.
    This is useful for uploading the data to the GPU vertex buffer object.
    */
    inline uint8_t * getData() const {
        return m_data;
    }

    /**
    Get a pointer to the indeces.
    This is useful for uploading the data to the GPU vertex buffer object.
    */
    inline uint32_t * getIndeces() const {
        return m_indeces;
    }

    /**
    Get a reference to a triangle group.
    You can use this reference to modify the internal data inside too.

    @param group The group index.
    */
    /*inline TriangleGroup& getTriangleGroup(uint8_t groupInd) const {
        return m_triangleGroups[groupInd];
    }*/

    /**
    Returns how many verteces are stored in the mesh.
    */
    inline uint32_t getNumVert() const {
        return m_numVert;
    }

    /**
    Returns how many triangles are stored in the mesh.
    */
    inline uint32_t getNumTri() const {
        return m_numTri;
    }

    /**
    Returns how many triangle groups are stored in the mesh.
    */
    /*inline uint8_t getNumGroups() const {
        return m_numGroups;
    }*/

private:
    uint32_t m_numVert;
    uint8_t * m_data;

    uint32_t m_numTri;
    uint32_t * m_indeces;

    //uint8_t m_numGroups;
    //TriangleGroup * m_triangleGroups;

    uint32_t m_positionOffset;
    uint32_t m_normalOffset;
    uint32_t m_tangentOffset;
    uint32_t m_bitangentOffset;
    uint32_t m_texCoordOffset;
    uint32_t m_blendIndexOffset;
    uint32_t m_blendWeightOffset;
    uint32_t m_colorOffset;

    uint8_t m_vertexSize;

    FeaturesMask m_features;
};

#endif
