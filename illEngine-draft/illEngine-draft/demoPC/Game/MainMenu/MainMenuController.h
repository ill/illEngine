#ifndef __MAIN_MENU_CONTROLLER_H__
#define __MAIN_MENU_CONTROLLER_H__

#include <map>
#include <glm/glm.hpp>

#include "../../GameControllerBase.h"
#include "../CameraController.h"

#include "serial-illGraphics/Camera/Camera.h"
#include "serial-illGraphics/Camera/CameraTransform.h"
#include "serial-illGraphics/Model/Mesh.h"
#include "serial-illGraphics/Material/Texture.h"
#include "serial-illGraphics/Model/Skeleton.h"
#include "serial-illGraphics/Model/SkeletonAnimation.h"
#include "serial-illGraphics/Model/ModelAnimationController.h"
#include "serial-illGraphics/Material/ShaderProgram.h"

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
    
    //marine

    Graphics::Mesh m_marine;
    Graphics::Mesh m_marineHelmet;
    Graphics::Texture m_marineDiffuse;
    Graphics::Texture m_helmetDiffuse;
    Graphics::Texture m_marineNormal;
    Graphics::Texture m_helmetNormal;
    Graphics::Skeleton m_marineSkeleton;
    Graphics::SkeletonAnimation m_marineAnimation;
    Graphics::ModelAnimationController m_marineController;

    //hell knight

    Graphics::Mesh m_hellKnight;
    Graphics::Texture m_hellKnightDiffuse;
    Graphics::Texture m_hellKnightNormal;
    Graphics::Skeleton m_hellKnightSkeleton;
    Graphics::SkeletonAnimation m_hellKnightAnimation;

    Graphics::ModelAnimationController m_hellKnightController0;
    Graphics::ModelAnimationController m_hellKnightController1;
    Graphics::ModelAnimationController m_hellKnightController2;

    //demon

    Graphics::Mesh m_demon;
    Graphics::Texture m_demonDiffuse;
    Graphics::Texture m_demonNormal;
    Graphics::Skeleton m_demonSkeleton;
    Graphics::SkeletonAnimation m_demonAnimation;

    Graphics::ModelAnimationController m_demonController0;
    Graphics::ModelAnimationController m_demonController1;
    Graphics::ModelAnimationController m_demonController2;
    Graphics::ModelAnimationController m_demonController3;

    //demon front

    Graphics::Mesh m_demonFront;

    //the skinning shader

    Graphics::ShaderProgram m_debugShader;
    Graphics::ShaderProgramLoader * m_debugShaderLoader;

    /*std::map<unsigned int, glm::mat4> m_animationTest;    //temporarily testing animations manually without the animation controller
    glm::mat4 * m_animationTestSkelMats;*/
};
}

#endif