#ifndef __MAIN_MENU_CONTROLLER_H__
#define __MAIN_MENU_CONTROLLER_H__

#include "../../GameControllerBase.h"
#include "../CameraController.h"

#include "../../../illGraphics-draft-singleThreaded/Camera/Camera.h"
#include "../../../illGraphics-draft-singleThreaded/Camera/CameraTransform.h"
#include "../../../illGraphics-draft-singleThreaded/Model/Mesh.h"
#include "../../../illGraphics-draft-singleThreaded/Model/Skeleton.h"
#include "../../../illGraphics-draft-singleThreaded/Material/ShaderProgram.h"

namespace Demo {
struct Engine;

class MainMenuController : public GameControllerBase {
public:
    MainMenuController(Engine * engine);
    virtual ~MainMenuController();

    void update(float seconds);
    void updateSound(float seconds);
    void render();

private:
    Engine * m_engine;

    CameraController m_cameraController;

    Graphics::Camera m_camera;
    Graphics::CameraTransform m_cameraTransform;
    Graphics::Mesh m_mesh;
    Graphics::ShaderProgram m_debugShader;
    Graphics::ShaderProgramLoader * m_debugShaderLoader;
    Graphics::Skeleton m_skeleton;
};
}

#endif