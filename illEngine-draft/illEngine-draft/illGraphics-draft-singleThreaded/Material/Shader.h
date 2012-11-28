#ifndef __SHADER_H__
#define __SHADER_H__

#include <cstdint>
#include <string>

#include "../../illUtil-draft-singleThreaded/ResourceBase.h"
#include "../../illUtil-draft-singleThreaded/ResourceManager.h"

namespace Graphics {

class RendererBackend;

class Shader : public ResourceBase<Shader, uint64_t, RendererBackend> {
public:
    enum Feature {
        SHADER_3D_VERT = 1 << 0,         ///<Doing a standard 3D vertex transform

        SHADER_FORWARD_FRAG = 1 << 1,    ///<Doing a fragment shader for a forward rendered object

        SHADER_POSITIONS = 1 << 2,       ///<Positions are being sent
        SHADER_NORMALS = 1 << 3,         ///<Normals are being sent
        SHADER_TEX_COORDS = 1 << 4,      ///<Texture coordinates are being sent
        SHADER_TANGENTS = 1 << 5,        ///<Texture coordinates are being sent

        //TODO colors and speculars

        SHADER_DIFFUSE_MAP = 1 << 6,     ///<Diffuse map is enabled
        SHADER_SPECULAR_MAP = 1 << 7,    ///<Specular map is enabled
        SHADER_EMISSIVE_MAP = 1 << 8,    ///<Emissive map is enabled
        SHADER_NORMAL_MAP = 1 << 9,      ///<Normal map is enabled

        SHADER_LIGHTING = 1 << 10,       ///<Forward Lighting is enabled
    };
    
    Shader()
        : ResourceBase(),
        m_shaderData(NULL)
    {}

    virtual ~Shader() {
        unload();
    }

    void unload();
    void reload(RendererBackend * rendererBackend);

    inline void * getShaderData() const {
        return m_shaderData; 
    }

    void loadInternal(RendererBackend * renderer, const std::string& path, unsigned int shaderType, const std::string& defines);

private:
    void* m_shaderData;
};

typedef ResourceManager<uint64_t, Shader, RendererBackend> ShaderManager;
}

#endif