#ifndef ILL_MATERIAL_H_
#define ILL_MATERIAL_H_

#include <glm/glm.hpp>

#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"
#include "Graphics/serial/Material/ShaderProgram.h"
#include "Graphics/serial/Material/Texture.h"

namespace illGraphics {

struct MaterialLoadInfo {
	enum class BlendMode {
		NONE,                           ///<solid object, no blending
		ALPHA,                          ///<alpha blended
		PREMULT_ALPHA,                  ///<premultiplied alpha blended
		ADDITIVE                        ///<additively blended
    };

    enum class BillboardMode {
        NONE,                           ///<no billboarding
        XY,                             ///<the object is billboarded to face the camera in the x,y plane only
        XYZ                             ///<the object is fully billboarded to face the camera
    };

    //TODO: use this?
    enum class Reflectance {
        PHONG,
        SKIN
    };

    int m_diffuseTextureIndex;		///<diffuse component texture
	glm::vec4 m_diffuseBlend;       ///<RGBA color to multiply the diffuse component with

    int m_specularTextureIndex;		///<specular component texture
    glm::vec4 m_specularBlend;      ///<RGB specular color to multiply and shininess coefficient

    int m_emissiveTextureIndex;		///<emissive component texture
    glm::vec4 m_emissiveBlend;

    int m_normalTextureIndex;		///<the normal texture
    float m_normalMultiplier;       ///<normal mapping depth multiplier
	
    BlendMode m_blendMode;
    BillboardMode m_billboardMode;

    bool m_noLighting;
    bool m_skinning;
    bool m_forceForwardRendering;

	//TODO: more to come, like detail textures, displacement maps, opacity maps, etc...
	//TODO: might make a material system with nodes eventually, that'll make shaders pretty complicated though
};

struct MaterialLoader {
    MaterialLoader(ShaderProgramManager * shaderProgramManager, TextureManager * textureManager)
        : m_shaderProgramManager(shaderProgramManager),
        m_textureManager(textureManager),
        m_forceForwardRendering(false)
    {}

    bool m_forceForwardRendering;
    ShaderProgramManager * m_shaderProgramManager;
    TextureManager * m_textureManager;
};

class Material : public ResourceBase<Material, MaterialLoadInfo, MaterialLoader> {
public:
	Material()
        : ResourceBase()
    {}

    virtual ~Material() {
        unload();
    }

    virtual void unload();
    virtual void reload(MaterialLoader * loader);

	inline const Texture * getDiffuseTexture() const {
        return m_diffuseTexture.get();
    }

    inline const Texture * getSpecularTexture() const {
        return m_specularTexture.get();
    }

    inline const Texture * getEmissiveTexture() const {
        return m_emissiveTexture.get();
    }

    inline const Texture * getNormalTexture() const {
        return m_normalTexture.get();
    }

    inline const ShaderProgram * getShaderProgram() const {
        return m_shaderProgram.get();
    }
	
private:
	RefCountPtr<Texture> m_diffuseTexture;
	RefCountPtr<Texture> m_specularTexture;
	RefCountPtr<Texture> m_emissiveTexture;
	RefCountPtr<Texture> m_normalTexture;

	RefCountPtr<ShaderProgram> m_shaderProgram;
};

}

#endif