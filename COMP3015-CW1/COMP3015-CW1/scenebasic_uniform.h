#pragma once

#include "helper/scene.h"
#include "helper/glslprogram.h"

#include <glm/glm.hpp>

#include <vector>
#include <string>

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog;

    GLSLProgram uiProg;
    GLuint uiVao = 0;
    GLuint uiVbo = 0;

    std::vector<float> uiRectVerts;
    std::vector<float> uiTextVerts;

    void compileUI();
    void initUI();
    void pushRect(float x, float y, float w, float h);
    void pushText(float x, float y, const std::string& text);
    void drawOverlay();

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

    bool isDarkMode = false;
    bool togglePressed = false;

    bool spotlightMode = false;
    bool fogMode = false;

    bool tPressed = false;
    bool fPressed = false;

    void compile();
    void buildCube();
    void buildGround();

    glm::vec3 lightPos = glm::vec3(0.0f, -100.0f, 0.0f);

    struct GuardPart {
        GLuint vao = 0;
        GLuint vbo = 0;
        int count = 0;
        glm::vec3 kd = glm::vec3(1.0f);
    };

    std::vector<GuardPart> guardParts;

public:
    SceneBasic_Uniform();

    void initScene() override;
    void update(float t) override;
    void render() override;
    void resize(int w, int h) override;

    GLuint guardVao = 0;
    GLuint guardVbo = 0;
    int guardVertexCount = 0;
};