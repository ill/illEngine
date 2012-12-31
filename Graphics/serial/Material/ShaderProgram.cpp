#include <GL/glew.h>

#include "ShaderProgram.h"
#include "Shader.h"
#include "Graphics/RendererBackend.h"

using namespace std;

namespace illGraphics {

void ShaderProgram::unload() {
    if(m_state == RES_LOADING) {
        LOG_FATAL_ERROR("Attempting to unload shader program while it's loading");
    }

    if(m_state == RES_UNINITIALIZED || m_state == RES_UNLOADED) {
        return;
    }

    m_loader->m_rendererBackend->unloadShaderProgram(&m_shaderProgramData);

    m_state = RES_UNLOADED;
}

void ShaderProgram::reload(ShaderProgramLoader * loader) {   
    unload();

    m_loader = loader;

    m_state = RES_LOADING;

    //figure out the shaders to load for the needed features
    uint64_t shaderId;

    //forward rendering
    if(m_loadArgs & SHPRG_FORWARD) {
        //the main vertex shader
        shaderId = Shader::SHADER_3D_VERT;

        if(m_loadArgs & SHPRG_POSITIONS) {
            shaderId |= Shader::SHADER_POSITIONS;
        }

        if(m_loadArgs & SHPRG_NORMALS) {
            shaderId |= Shader::SHADER_NORMALS;
        }

        if(m_loadArgs & SHPRG_DIFFUSE_MAP || m_loadArgs & SHPRG_SPECULAR_MAP || m_loadArgs & SHPRG_EMISSIVE_MAP) {
            shaderId |= Shader::SHADER_TEX_COORDS;
        }

        if(m_loadArgs & SHPRG_NORMAL_MAP) {
            shaderId |= Shader::SHADER_TEX_COORDS;
            shaderId |= Shader::SHADER_TANGENTS;
        }

        m_shaders.push_back(m_loader->m_shaderManager->getResource(shaderId));

        //the forward fragment shader
        shaderId = Shader::SHADER_FORWARD_FRAG;

        if(m_loadArgs & SHPRG_FORWARD_LIGHT) {
            shaderId |= Shader::SHADER_LIGHTING;
        }

        if(m_loadArgs & SHPRG_DIFFUSE_MAP) {
            shaderId |= Shader::SHADER_DIFFUSE_MAP;
        }

        if(m_loadArgs & SHPRG_SPECULAR_MAP) {
            shaderId |= Shader::SHADER_SPECULAR_MAP;
        }

        if(m_loadArgs & SHPRG_EMISSIVE_MAP) {
            shaderId |= Shader::SHADER_EMISSIVE_MAP;
        }

        if(m_loadArgs & SHPRG_NORMAL_MAP) {
            shaderId |= Shader::SHADER_NORMAL_MAP;
        }

        m_shaders.push_back(m_loader->m_shaderManager->getResource(shaderId));
    }

    build();
}

void ShaderProgram::build() {
    m_loader->m_rendererBackend->loadShaderProgram(&m_shaderProgramData, m_shaders);

    m_state = RES_LOADED;
}

}
