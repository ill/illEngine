#include <set>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../../Engine.h"
#include "../../../illGraphics-draft-singleThreaded/Window.h"

#include "MainMenuController.h"
//#include "../../../illUtil-draft-singleThreaded/ObjLoader.h"
#include "../../../illUtil-draft-singleThreaded/Illmesh/IllmeshLoader.h"
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
    /*ObjLoader<> objLoader("Meshes/hellKnight.obj", glm::vec3(0.01f), MF_POSITION | MF_NORMAL | MF_TANGENT | MF_BLEND_DATA | MF_TEX_COORD);
    objLoader.parse();

    m_mesh.m_meshFrontendData = new MeshData<>(objLoader.m_numTri, objLoader.m_numVert, (uint8_t) objLoader.m_triangleGroupIndeces.size(), MF_POSITION | MF_NORMAL | MF_TANGENT | MF_BLEND_DATA | MF_TEX_COORD);

    objLoader.buildMesh(*m_mesh.m_meshFrontendData);
    m_mesh.frontendBackendTransfer(m_engine->m_rendererBackend);*/

    {
        IllmeshLoader<> meshLoader("Meshes/boblamp.illmesh");

        m_mesh.m_meshFrontendData = new MeshData<>(meshLoader.m_numInd / 3, meshLoader.m_numVert, meshLoader.m_features);
    
        meshLoader.buildMesh(*m_mesh.m_meshFrontendData);
        m_mesh.frontendBackendTransfer(m_engine->m_rendererBackend);
    }

    //load the skeleton
    {
        Graphics::SkeletonLoadArgs loadArgs;
        loadArgs.m_path = "Meshes/boblamp.illskel";
        m_skeleton.load(loadArgs, NULL);
    }

    //load the animation
    {
        Graphics::SkeletonAnimationLoadArgs loadArgs;
        loadArgs.m_path = "Meshes/boblamp.illanim";
        m_animation.load(loadArgs, NULL);
    }

    //load the test shader
    {
        std::vector<RefCountPtr<Graphics::Shader> > shaders;

        Graphics::Shader * shader = new Graphics::Shader();
        shader->loadInternal(m_engine->m_rendererBackend, "shaders/debugShader.vert", GL_VERTEX_SHADER, "");

        shaders.push_back(RefCountPtr<Graphics::Shader>(shader));

        shader = new Graphics::Shader();
        shader->loadInternal(m_engine->m_rendererBackend, "shaders/debugShader.frag", GL_FRAGMENT_SHADER, "");

        shaders.push_back(RefCountPtr<Graphics::Shader>(shader));

        m_debugShaderLoader = new Graphics::ShaderProgramLoader(m_engine->m_rendererBackend, NULL);
        m_debugShader.loadInternal(m_debugShaderLoader, shaders);
    }

    //initialize the input (this would normally initialize using console variables)
    m_engine->m_inputManager->addPlayer(0);
    m_engine->m_inputManager->bindDevice(SdlPc::PC_KEYBOARD, 0);
    m_engine->m_inputManager->bindDevice(SdlPc::PC_MOUSE, 0);
    m_engine->m_inputManager->bindDevice(SdlPc::PC_MOUSE_BUTTON, 0);
    m_engine->m_inputManager->bindDevice(SdlPc::PC_MOUSE_WHEEL, 0);

    m_engine->m_inputManager->getInputContextStack(0)->pushInputContext(&m_cameraController.m_inputContext);

    m_cameraController.m_speed = 10.0f;
    m_cameraController.m_rollSpeed = 50.0f;
}

MainMenuController::~MainMenuController() {
    delete m_debugShaderLoader;
}

void MainMenuController::update(float seconds) {
    m_cameraController.update(seconds);

    static float animTime = 0.0f;

    animTime += seconds;

    const std::map<std::string, Graphics::SkeletonAnimation::Transform*>& animations = m_animation.getAnimations();

    Graphics::SkeletonAnimation::InterpInfo interpInfo = m_animation.getFrames(animTime);

    //get transforms for all the bones
    for(std::map<std::string, Graphics::SkeletonAnimation::Transform*>::const_iterator iter = animations.begin(); iter != animations.end(); iter++) {
        Graphics::SkeletonAnimation::Transform* transforms = iter->second;

        Graphics::SkeletonAnimation::Transform currentTransform;

        currentTransform.m_position = glm::mix(transforms[interpInfo.m_frame1].m_position, transforms[interpInfo.m_frame2].m_position, interpInfo.m_delta);
        currentTransform.m_rotation = glm::fastMix(transforms[interpInfo.m_frame1].m_rotation, transforms[interpInfo.m_frame2].m_rotation, interpInfo.m_delta);
        currentTransform.m_scale = glm::mix(transforms[interpInfo.m_frame1].m_scale, transforms[interpInfo.m_frame2].m_scale, interpInfo.m_delta);

        //compute the matrix
        glm::mat4 transform = glm::translate(currentTransform.m_position);
        transform = glm::mat4_cast(currentTransform.m_rotation) * transform;
        transform = glm::scale(transform, currentTransform.m_scale);
        /*glm::mat4 transform = glm::scale(currentTransform.m_scale);
        transform = glm::mat4_cast(currentTransform.m_rotation) * transform;
        transform = glm::translate(transform, currentTransform.m_position);*/

        //glm::mat4 transform = glm::scale(currentTransform.m_scale) * glm::mat4_cast(currentTransform.m_rotation) * glm::translate(currentTransform.m_position);

        /*glm::mat4 transform = glm::mat4_cast(currentTransform.m_rotation);
        transform[0][0] *= currentTransform.m_scale.x;
        transform[0][1] *= currentTransform.m_scale.x;
        transform[0][2] *= currentTransform.m_scale.x;

        transform[1][0] *= currentTransform.m_scale.y;
        transform[1][1] *= currentTransform.m_scale.y;
        transform[1][2] *= currentTransform.m_scale.y;

        transform[2][0] *= currentTransform.m_scale.z;
        transform[2][1] *= currentTransform.m_scale.z;
        transform[2][2] *= currentTransform.m_scale.z;

        transform[3][0] = currentTransform.m_position.x;
        transform[3][1] = currentTransform.m_position.y;
        transform[3][2] = currentTransform.m_position.z;*/
        
        //place the transform into the thing
        m_animationTest[m_skeleton.getBone(iter->first)] = transform;
    }
}

void MainMenuController::updateSound(float seconds) {

}

void debugDrawSkeleton(const Graphics::Skeleton::BoneHeirarchy * currNode, glm::mat4 currXform, const glm::vec3& lastPos, const std::map<const Graphics::Skeleton::Bone *, glm::mat4> animTransforms) {
    //if the bone is in the animTransform list, use that instead
    const std::map<const Graphics::Skeleton::Bone *, glm::mat4>::const_iterator iter = animTransforms.find(currNode->m_bone);
    
    if(iter != animTransforms.end()) {
        currXform = currXform * iter->second;
    }
    else {
        currXform = currXform * currNode->m_bone->m_transform;
    }

    glm::vec4 currPoint(0.0f, 0.0f, 0.0f, 1.0f);
    currPoint = currXform * currPoint;

    //draw line from this bone to the last bone
    if(currNode->m_parent) {
        glColor3f(0.0f, 0.0f, 1.0f);

        glBegin(GL_LINES);
            glVertex3fv(glm::value_ptr(lastPos));
            glVertex3fv(glm::value_ptr(currPoint));
        glEnd();
    }

    glPointSize(5.0f);
    glColor3f(1.0f, 0.0f, 0.0f);

    //draw the bone point
    glBegin(GL_POINTS);
    glVertex3fv(glm::value_ptr(currPoint));
    glEnd();

    //draw the bone line
    glColor3f(0.0f, 1.0f, 0.0f);

    glm::vec3 thisPoint = glm::vec3(currPoint);

    glBegin(GL_LINES);
    glVertex3fv(glm::value_ptr(currPoint));

    currPoint = glm::vec4(0.0f, 5.0f, 0.0f, 1.0f);
    currPoint = currXform * currPoint;

    glVertex3fv(glm::value_ptr(currPoint));
    glEnd();

    for(std::vector<Graphics::Skeleton::BoneHeirarchy *>::const_iterator iter = currNode->m_children.begin(); iter != currNode->m_children.end(); iter++) {
        debugDrawSkeleton(*iter, currXform, thisPoint, animTransforms);
    }    
}

void MainMenuController::render() {
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    m_cameraTransform.m_transform = m_cameraController.m_transform;
    m_camera.setTransform(m_cameraTransform, m_engine->m_window->getAspectRatio(), Graphics::DEFAULT_FOV * m_cameraController.m_zoom);

    //TODO: for now I'm testing a bunch of stuff, normally all rendering is done through the renderer   
    //render the lil mesh
    //glEnable(GL_DEPTH_TEST);
    GLuint prog = *((GLuint *) m_debugShader.getShaderProgram());

    glUseProgram(prog);

    GLint loc = glGetUniformLocation(prog, "modelViewProjectionMatrix");
    if(loc == -1) {
        LOG_FATAL_ERROR("Unknown uniform");
    }
    glUniformMatrix4fv(loc, 1, false, glm::value_ptr(m_camera.getCanonical()));

    /*loc = glGetUniformLocation(prog, "normalMatrix");
    if(loc == -1) {
        LOG_FATAL_ERROR("Unknown uniform");
    }
    glUniformMatrix3fv(loc, 1, false, glm::value_ptr(m_camera.getNormal()));*/

    GLuint buffer = *((GLuint *) m_mesh.getMeshBackendData() + 0);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    loc = glGetAttribLocation(prog, "position");
    if(loc == -1) {
        LOG_FATAL_ERROR("Unknown attrib");
    }
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, m_mesh.m_meshFrontendData->getVertexSize(), (char *)NULL + m_mesh.m_meshFrontendData->getPositionOffset());
    glEnableVertexAttribArray(loc);

    /*loc = glGetAttribLocation(prog, "normal");
    if(loc == -1) {
        LOG_FATAL_ERROR("Unknown attrib");
    }
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE,  m_mesh.m_meshFrontendData->getVertexSize(), (char *)NULL + m_mesh.m_meshFrontendData->getNormalOffset());
    glEnableVertexAttribArray(loc);*/

    buffer = *((GLuint *) m_mesh.getMeshBackendData() + 1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);

    glDrawRangeElements(GL_TRIANGLES, 0, m_mesh.m_meshFrontendData->getNumTri() * 3, m_mesh.m_meshFrontendData->getNumTri() * 3, GL_UNSIGNED_INT, (char *)NULL);

    //debug drawing
    glUseProgram(0);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(m_camera.getProjection()));

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(m_camera.getModelView()));

    //debug draw the axes
    glBegin(GL_LINES);
    //x
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(5.0f, 0.0f, 0.0f);

    //y
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 5.0f, 0.0f);

    //z
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 5.0f);
    glEnd();

    //debug draw the skeleton
    debugDrawSkeleton(m_skeleton.getRootBoneNode(), glm::mat4(), glm::vec3(0.0f), m_animationTest);

    ERROR_CHECK_OPENGL;
}

}