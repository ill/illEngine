#include "Graphics/serial/Material/Material.h"
#include "Graphics/GraphicsBackend.h"

#include "Logging/logging.h"

namespace illGraphics {

void Material::unload() {
    if(m_state == RES_LOADING) {
        LOG_FATAL_ERROR("Attempting to unload material while it's loading");
    }

    if(m_state == RES_UNINITIALIZED || m_state == RES_UNLOADED) {
        return;
    }

	m_diffuseTexture.reset();
	m_specularTexture.reset();
	m_emissiveTexture.reset();
	m_normalTexture.reset();

	m_shaderProgram.reset();

    m_state = RES_UNLOADED;
}

void Material::reload(MaterialLoader * rendererBackend) {
    unload();

    m_loader = rendererBackend;

    m_state = RES_LOADING;

    uint64_t shaderMask = ShaderProgram::SHPRG_POSITIONS;

    //load textures
    //TODO: check if textures are effectively unused if things like blend colors make them invisible

    bool normalsNeeded = false;

    if(m_loadArgs.m_diffuseTextureIndex >= 0) {
        m_diffuseTexture = m_loader->m_textureManager->getResource(m_loadArgs.m_diffuseTextureIndex);
        shaderMask |= ShaderProgram::SHPRG_DIFFUSE_MAP;
        normalsNeeded = true;
    }
        
    if(m_loadArgs.m_emissiveTextureIndex >= 0) {
        m_emissiveTexture = m_loader->m_textureManager->getResource(m_loadArgs.m_emissiveTextureIndex);
        shaderMask |= ShaderProgram::SHPRG_EMISSIVE_MAP;
    }
        
    if(m_loadArgs.m_specularTextureIndex >= 0 && !m_loadArgs.m_noLighting) {
        m_specularTexture = m_loader->m_textureManager->getResource(m_loadArgs.m_specularTextureIndex);
        shaderMask |= ShaderProgram::SHPRG_SPECULAR_MAP;
        normalsNeeded = true;
    }

    if(m_loadArgs.m_normalTextureIndex >= 0 && !m_loadArgs.m_noLighting) {
        m_normalTexture = m_loader->m_textureManager->getResource(m_loadArgs.m_normalTextureIndex);
        shaderMask |= ShaderProgram::SHPRG_NORMAL_MAP;
        normalsNeeded = true;
    }

    //if any textures are used that may benefit from normals and no lighting isn't on
    if(normalsNeeded && !m_loadArgs.m_noLighting) {
        shaderMask |= ShaderProgram::SHPRG_NORMALS;
    }

    //check if should do forward rendering instead of deferred shading
    if(m_loader->m_forceForwardRendering || m_loadArgs.m_forceForwardRendering || m_loadArgs.m_blendMode != MaterialLoadArgs::BlendMode::NONE) {
        shaderMask |= ShaderProgram::SHPRG_FORWARD;

        if(!m_loadArgs.m_noLighting) {
            shaderMask |= ShaderProgram::SHPRG_FORWARD_LIGHT;
        }
    }
        
    //TODO: billboarding mode

    if(m_loadArgs.m_skinning) {
        shaderMask |= ShaderProgram::SHPRG_SKINNING;
    }

    //load defferred g buffer shader
    m_shaderProgram = m_loader->m_shaderProgramManager->getResource(shaderMask);
    
    m_state = RES_LOADED;
}

}