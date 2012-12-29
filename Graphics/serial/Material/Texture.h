#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <string>
#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"

namespace Graphics {
    
class RendererBackend;

struct TextureLoadArgs {
    std::string m_path;  //path of image file

    int m_wrapS;         //wrap attribute value
    int m_wrapT;         //wrap attribute value
};

/**
Textures are used by materials for drawing 3D objects.
Sometimes a texture might be useable by itself outside of a material for drawing HUD elements or something.
*/
class Texture : public ResourceBase<Texture, TextureLoadArgs, RendererBackend> {
public:
    Texture()
        : ResourceBase(),
        m_textureData(NULL)
    {}

    virtual ~Texture() {
        unload();
    }

    virtual void unload();
    virtual void reload(RendererBackend * renderer);

    inline void* getTextureData() const {
        return m_textureData; 
    }

private:
    void* m_textureData;
};

typedef ConfigurableResourceManager<uint32_t, Texture, TextureLoadArgs, RendererBackend> TextureManager;
}

#endif
