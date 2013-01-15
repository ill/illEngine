#ifndef ILL_GL_RENDERER_H__
#define ILL_GL_RENDERER_H__

//#include <GL/glew.h>
//#include <set>

#include "Graphics/RendererBackend.h"
#include "Graphics/serial/Material/ShaderProgram.h"         //temporary for now until I get the material system working again

namespace GlCommon {

class GlRenderer : public illGraphics::RendererBackend {
public:
    void initialize();
    void uninitialize();

    void beginFrame();
    void endFrame();

    //void beginFontRender(const illGraphics::BitmapFont &font);
    //void renderCharacter(const illGraphics::Camera& camera, const glm::mat4& transform, const illGraphics::BitmapFont &font, unsigned char character);

    ////////////////////
    //Resource loading functions
    void loadTexture(void ** textureData, const illGraphics::TextureLoadArgs& loadArgs);
    void unloadTexture(void ** textureData);
    
    void loadMesh(void** meshBackendData, const MeshData<>& meshFrontendData);
    void unloadMesh(void** meshBackendData);

    void loadShader(void ** shaderData, uint64_t featureMask);
    void loadShaderInternal(void ** shaderData, const char * path, unsigned int shaderType, const char * defines);
    void unloadShader(void ** shaderData);

    void loadShaderProgram(void ** programData, const std::vector<RefCountPtr<illGraphics::Shader> >& shaderList);
    void unloadShaderProgram(void ** programData);

    ////////////////////
    //GL State Stuff
    /*inline void setShaderProgram(GLuint program) {
        if(m_currentShaderProgram != program) {
            m_currentShaderProgram = program;
            glUseProgram(m_currentShaderProgram);
        }
    }

    inline GLuint getShaderProgram() const {
        return m_currentShaderProgram;
    }

    inline void enableVertexAttribArray(GLint loc) {
        if(!checkEnabledVertexAttribArray(loc)) {
            glDisableVertexAttribArray(loc);
            m_enabledVertexAttribArrays.insert(loc);
        }
    }

    inline void disableVertexAttribArray(GLint loc) {
        std::set<GLint>::iterator iter = m_enabledVertexAttribArrays.find(loc);

        if(iter != m_enabledVertexAttribArrays.end()) {
            glDisableVertexAttribArray(*iter);
            m_enabledVertexAttribArrays.erase(iter);
        }
    }

    inline bool checkEnabledVertexAttribArray(GLint loc) const {
        std::set<GLint>::iterator iter = m_enabledVertexAttribArrays.find(loc);

        return iter != m_enabledVertexAttribArrays.end();
    }

    inline void disableAllVertexAttribArrays() {
        for(std::set<GLint>::iterator iter = m_enabledVertexAttribArrays.begin(); iter != m_enabledVertexAttribArrays.end(); iter++) {
            glDisableVertexAttribArray(*iter);
        }

        m_enabledVertexAttribArrays.clear();
    }

private:

    GLuint m_currentShaderProgram;
    std::set<GLint> m_enabledVertexAttribArrays;*/

private:

    illGraphics::ShaderProgram m_fontShader;                //temporary for now until I get the material system working again
    illGraphics::ShaderProgramLoader * m_debugShaderLoader;
};

}

#endif
