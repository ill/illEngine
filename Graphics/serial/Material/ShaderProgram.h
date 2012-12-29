#ifndef __SHADER_PROGRAM_H__
#define __SHADER_PROGRAM_H__

#include <cstdint>
#include <vector>

#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"
#include "Shader.h"

namespace Graphics {

class RendererBackend;

typedef ResourceManager<uint64_t, Shader, RendererBackend> ShaderManager;

struct ShaderProgramLoader {
    ShaderProgramLoader(RendererBackend * rendererBackend, ShaderManager * shaderManager)
        : m_rendererBackend(rendererBackend),
        m_shaderManager(shaderManager)
    {}

    RendererBackend * m_rendererBackend;
    ShaderManager * m_shaderManager;
};

class ShaderProgram : public ResourceBase<ShaderProgram, uint64_t, ShaderProgramLoader> {
public:
    enum Feature {
        SHPRG_FORWARD = 1 << 0,             ///<Doing a forward render
        SHPRG_FORWARD_LIGHT = 1 << 1,       ///<Forward lighting is enabled

        SHPRG_POSITIONS = 1 << 2,           ///<Positions are being sent
        SHPRG_NORMALS = 1 << 3,             ///<Normals are being sent

        SHPRG_DIFFUSE_MAP = 1 << 4,         ///<Diffuse map is enabled, implies texture coordinates are sent
        SHPRG_SPECULAR_MAP = 1 << 5,        ///<Specular map is enabled, implies texture coordinates are sent
        SHPRG_EMISSIVE_MAP = 1 << 6,        ///<Emissive map is enabled, implies texture coordinates are sent
        SHPRG_NORMAL_MAP = 1 << 7,          ///<Normal map is enabled, implies texture coordinates and tangents are sent
    };

    ShaderProgram()
        : ResourceBase(),
        m_shaderProgramData(NULL) 
    {}

    virtual ~ShaderProgram() {
        unload();
    }

    void unload();
    void reload(ShaderProgramLoader * loader);

    inline void loadInternal(ShaderProgramLoader * loader, const std::vector<RefCountPtr<Shader> >& shaderList) {
        unload();

        m_loader = loader;

        m_state = RES_LOADING;

        m_shaders = shaderList;

        build();
    }

    inline void * getShaderProgram() const { 
        return m_shaderProgramData; 
    }
        
private:
    void build();

    std::vector<RefCountPtr<Shader> > m_shaders;
    void * m_shaderProgramData;
};

typedef ResourceManager<uint64_t, ShaderProgram, ShaderProgramLoader> ShaderProgramManager;
}

#endif
