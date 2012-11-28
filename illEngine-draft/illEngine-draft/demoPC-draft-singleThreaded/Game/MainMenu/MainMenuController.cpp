#include <set>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../../Engine.h"
#include "../../../illGraphics-draft-singleThreaded/Window.h"

#include "MainMenuController.h"
#include "../../../illUtil-draft-singleThreaded/ObjLoader.h"
#include "../../../illGraphics-draft-singleThreaded/Material/Shader.h"
#include "../../../illInput-draft-singleThreaded/InputManager.h"

#include "../../../illGlCommon-draft-singleThreaded/glLogging.h"

//TODO: for now I'm testing a bunch of stuff, normally all rendering is done through the renderer
#include <GL/glew.h>

namespace Demo {

MainMenuController::MainMenuController(Engine * engine)
    : GameControllerBase(),
    m_engine(engine)
{
    //This is all put together to test some stuff, this is in no way how to normally do these things.  Everything should normally be done through the renderer front end when that's done.

    //load the test mesh
    ObjLoader<> objLoader("Meshes/hellKnight.obj", glm::vec3(0.01f), MF_POSITION | MF_NORMAL | MF_TANGENT | MF_BLEND_DATA | MF_TEX_COORD);
    objLoader.parse();

    m_mesh.m_meshFrontendData = new MeshData<>(objLoader.m_numTri, objLoader.m_numVert, (uint8_t) objLoader.m_triangleGroupIndeces.size(), MF_POSITION | MF_NORMAL | MF_TANGENT | MF_BLEND_DATA | MF_TEX_COORD);

    objLoader.buildMesh(*m_mesh.m_meshFrontendData);
    m_mesh.frontendBackendTransfer(m_engine->m_rendererBackend);

    //load the test shader
    std::vector<RefCountPtr<Graphics::Shader> > shaders;

    Graphics::Shader * shader = new Graphics::Shader();
    shader->loadInternal(m_engine->m_rendererBackend, "shaders/debugShader.vert", GL_VERTEX_SHADER, "");

    shaders.push_back(RefCountPtr<Graphics::Shader>(shader));

    shader = new Graphics::Shader();
    shader->loadInternal(m_engine->m_rendererBackend, "shaders/debugShader.frag", GL_FRAGMENT_SHADER, "");

    shaders.push_back(RefCountPtr<Graphics::Shader>(shader));

    m_debugShaderLoader = new Graphics::ShaderProgramLoader(m_engine->m_rendererBackend, NULL);
    m_debugShader.loadInternal(m_debugShaderLoader, shaders);

    m_engine->m_inputManager->addPlayer(0);
    m_engine->m_inputManager->bindDevice(SdlPc::PC_KEYBOARD, 0);
    m_engine->m_inputManager->bindDevice(SdlPc::PC_MOUSE, 0);
    m_engine->m_inputManager->bindDevice(SdlPc::PC_MOUSE_BUTTON, 0);
    m_engine->m_inputManager->bindDevice(SdlPc::PC_MOUSE_WHEEL, 0);

    m_engine->m_inputManager->getInputContextStack(0)->pushInputContext(&m_cameraController.m_inputContext);
}

MainMenuController::~MainMenuController() {
    delete m_debugShaderLoader;
}

void MainMenuController::update(float seconds) {
    m_cameraController.update(seconds);
}

void MainMenuController::updateSound(float seconds) {

}

void MainMenuController::render() {
    m_cameraTransform.m_transform = m_cameraController.m_transform;
    m_camera.setTransform(m_cameraTransform, m_engine->m_window->getAspectRatio(), Graphics::DEFAULT_FOV * m_cameraController.m_zoom);

    //TODO: for now I'm testing a bunch of stuff, normally all rendering is done through the renderer   
    //render the lil mesh
    glEnable(GL_DEPTH_TEST);
    GLuint prog = *((GLuint *) m_debugShader.getShaderProgram());

    glUseProgram(prog);

    GLint loc = glGetUniformLocation(prog, "modelViewProjectionMatrix");
    if(loc == -1) {
        LOG_FATAL_ERROR("Unknown uniform");
    }
    glUniformMatrix4fv(loc, 1, false, glm::value_ptr(m_camera.getModelView()));

    loc = glGetUniformLocation(prog, "normalMatrix");
    if(loc == -1) {
        LOG_FATAL_ERROR("Unknown uniform");
    }
    glUniformMatrix3fv(loc, 1, false, glm::value_ptr(m_camera.getNormal()));

    GLuint buffer = *((GLuint *) m_mesh.getMeshBackendData() + 0);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    loc = glGetAttribLocation(prog, "position");
    if(loc == -1) {
        LOG_FATAL_ERROR("Unknown attrib");
    }
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, m_mesh.m_meshFrontendData->getVertexSize(), (char *)NULL + m_mesh.m_meshFrontendData->getPositionOffset());
    glEnableVertexAttribArray(loc);

    loc = glGetAttribLocation(prog, "normal");
    if(loc == -1) {
        LOG_FATAL_ERROR("Unknown attrib");
    }
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE,  m_mesh.m_meshFrontendData->getVertexSize(), (char *)NULL + m_mesh.m_meshFrontendData->getNormalOffset());
    glEnableVertexAttribArray(loc);

    buffer = *((GLuint *) m_mesh.getMeshBackendData() + 1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);

    glDrawRangeElements(GL_TRIANGLES, 0, m_mesh.m_meshFrontendData->getNumTri() * 3, m_mesh.m_meshFrontendData->getNumTri() * 3, GL_UNSIGNED_INT, (char *)NULL);

    ERROR_CHECK_OPENGL;
}

}