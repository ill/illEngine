#ifndef ILL_SHADER_H__
#define ILL_SHADER_H__

#include <stdint.h>

#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"

namespace illGraphics {

class GraphicsBackend;

class Shader : public ResourceBase<Shader, uint64_t, GraphicsBackend> {
public:
    enum Feature {
        SHADER_3D_VERT = 1 << 0,            ///<Doing a standard 3D vertex transform

        SHADER_FORWARD_FRAG = 1 << 1,       ///<Doing a fragment shader for a forward rendered object
        SHADER_DEFERRED_FRAG = 1 << 2,      ///<Doing a fragment shader for a deferred shaded object

        SHADER_POSITIONS = 1 << 3,          ///<Positions are being sent
        SHADER_NORMALS = 1 << 4,            ///<Normals are being sent
        SHADER_TEX_COORDS = 1 << 5,         ///<Texture coordinates are being sent
        SHADER_TANGENTS = 1 << 6,           ///<Texture coordinates are being sent
		SHADER_SKINNING = 1 << 7,           ///<Skeletal animation skinning data is being sent

        //TODO colors and speculars

        SHADER_DIFFUSE_MAP = 1 << 8,        ///<Diffuse map is enabled
        SHADER_SPECULAR_MAP = 1 << 9,       ///<Specular map is enabled
        SHADER_EMISSIVE_MAP = 1 << 10,       ///<Emissive map is enabled
        SHADER_NORMAL_MAP = 1 << 11,        ///<Normal map is enabled

        SHADER_LIGHTING = 1 << 12,          ///<Forward Lighting is enabled
    };
    
    Shader()
        : ResourceBase(),
        m_shaderData(NULL)
    {}

    virtual ~Shader() {
        unload();
    }

    void unload();
    void reload(GraphicsBackend * backend);

    inline void * getShaderData() const {
        return m_shaderData; 
    }

    void loadInternal(GraphicsBackend * backend, const char * path, unsigned int shaderType, const char * defines);

private:
    void* m_shaderData;
};

typedef ResourceManager<uint64_t, Shader, GraphicsBackend> ShaderManager;
}

#endif
