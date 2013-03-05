#ifndef ILL_GL_BACKEND_H_
#define ILL_GL_BACKEND_H_

//#include <GL/glew.h>
//#include <set>

#include "Graphics/GraphicsBackend.h"
#include "Graphics/serial/Material/ShaderProgram.h"         //temporary for now until I get the material system working again

namespace GlCommon {

class GlBackend : public illGraphics::GraphicsBackend {
public:
    virtual void initialize();
    virtual void uninitialize();

    virtual void beginFrame();
    virtual void endFrame();

    //void beginFontRender(const illGraphics::BitmapFont &font);
    //void renderCharacter(const illGraphics::Camera& camera, const glm::mat4& transform, const illGraphics::BitmapFont &font, unsigned char character);

    ////////////////////
    //Resource loading functions
    virtual void loadTexture(void ** textureData, const illGraphics::TextureLoadArgs& loadArgs);
    virtual void unloadTexture(void ** textureData);
    
    virtual void loadMesh(void** meshBackendData, const MeshData<>& meshFrontendData);
    virtual void unloadMesh(void** meshBackendData);

    virtual void loadShader(void ** shaderData, uint64_t featureMask);
    virtual void loadShaderInternal(void ** shaderData, const char * path, unsigned int shaderType, const char * defines);
    virtual void unloadShader(void ** shaderData);

    virtual void loadShaderProgram(void ** programData, const std::vector<RefCountPtr<illGraphics::Shader> >& shaderList);
    virtual void unloadShaderProgram(void ** programData);

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
