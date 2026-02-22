#pragma once

#include "helper/scene.h"
#include "helper/glslprogram.h"

#include <glm/glm.hpp>

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog;

    GLuint cubeVao = 0;
    GLuint groundVao = 0;

    GLuint floorTex = 0;

    GLuint cubeTex = 0;

    float angle = 0.0f;

    // Camera
    glm::vec3 camPos = glm::vec3(0.0f, 1.2f, 4.0f);
    glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float yaw = -90.0f;
    float pitch = 0.0f;

    double lastX = 400.0;
    double lastY = 300.0;
    bool firstMouse = true;

    float moveSpeed = 3.5f;
    float mouseSensitivity = 0.12f;

    float lastTime = 0.0f;

    bool isDarkMode = true;
    bool togglePressed = false;

    void compile();
    void buildCube();
    void buildGround();

public:
    SceneBasic_Uniform();

    void initScene() override;
    void update(float t) override;
    void render() override;
    void resize(int w, int h) override;
};
