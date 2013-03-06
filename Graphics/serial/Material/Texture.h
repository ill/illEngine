#ifndef ILL_TEXTURE_H__
#define ILL_TEXTURE_H__

#include <string>
#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"

namespace illGraphics {
    
class GraphicsBackend;

struct TextureLoadArgs {
    std::string m_path;  //path of image file

    enum class Wrap {
        W_CLAMP_TO_EDGE,
        W_REPEAT
    };

    Wrap m_wrapS;         //wrap attribute value
    Wrap m_wrapT;         //wrap attribute value
};

/**
Textures are used by materials for drawing 3D objects.
Sometimes a texture might be useable by itself outside of a material for drawing HUD elements or something.
*/
class Texture : public ResourceBase<TextureLoadArgs, GraphicsBackend> {
public:
    Texture()
        : ResourceBase(),
        m_textureData(NULL)
    {}

    virtual ~Texture() {
        unload();
    }

    virtual void unload();
    virtual void reload(GraphicsBackend * backend);

    inline void* getTextureData() const {
        return m_textureData; 
    }

private:
    void* m_textureData;
};

typedef uint32_t TextureId;
typedef ConfigurableResourceManager<TextureId, Texture, TextureLoadArgs, GraphicsBackend> TextureManager;
}

#endif
