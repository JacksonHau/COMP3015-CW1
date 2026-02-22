#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "helper/glutils.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using glm::vec3;

SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f) {}

static GLuint loadTexture2D(const char* path)
{
    int w, h, n;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &n, 4); // force RGBA
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLuint tex = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &tex);
    glTextureStorage2D(tex, 1, GL_RGBA8, w, h);
    glTextureSubImage2D(tex, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // Filtering
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Repeat so it tiles across your 0..10 UVs
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);
    return tex;
}

void SceneBasic_Uniform::initScene()
{
    compile();

    // Depth testing for real 3D occlusion
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearColor(0.62f, 0.70f, 0.85f, 1.0f);

    // Lock mouse for FPS camera
    if (window) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    buildCube();
    buildGround();

    floorTex = loadTexture2D("assets/wood.png");
    cubeTex = loadTexture2D("assets/brick.jpg");

    // initial projection
    projection = glm::perspective(glm::radians(60.0f), float(width) / float(height), 0.1f, 200.0f);
}

void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        prog.link();
        prog.use();
    }
    catch (GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::buildCube()
{
    // 36 vertices cube
    static const float positionData[] = {
        // +Z
        -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
        -0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        // -Z
        0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f, 0.5f,-0.5f,
        0.5f,-0.5f,-0.5f, -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
        // +X
        0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
        0.5f,-0.5f, 0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
        // -X
        -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f,-0.5f,-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f,
        // +Y
        -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f,
        -0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f,
        // -Y
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f
    };

    static const float normalData[] = {
        // +Z
        0,0,1, 0,0,1, 0,0,1, 0,0,1, 0,0,1, 0,0,1,
        // -Z
        0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1,
        // +X
        1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0,
        // -X
        -1,0,0, -1,0,0, -1,0,0, -1,0,0, -1,0,0, -1,0,0,
        // +Y
        0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0,
        // -Y
        0,-1,0, 0,-1,0, 0,-1,0, 0,-1,0, 0,-1,0, 0,-1,0
    };

    static const float uvData[] = {
        0,0, 1,0, 1,1, 0,0, 1,1, 0,1,
        0,0, 1,0, 1,1, 0,0, 1,1, 0,1,
        0,0, 1,0, 1,1, 0,0, 1,1, 0,1,
        0,0, 1,0, 1,1, 0,0, 1,1, 0,1,
        0,0, 1,0, 1,1, 0,0, 1,1, 0,1,
        0,0, 1,0, 1,1, 0,0, 1,1, 0,1
    };

    GLuint vbo[3];
    glGenBuffers(3, vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positionData), positionData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normalData), normalData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvData), uvData, GL_STATIC_DRAW);

    glGenVertexArrays(1, &cubeVao);
    glBindVertexArray(cubeVao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

#ifdef __APPLE__
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
#else
    glBindVertexBuffer(0, vbo[0], 0, sizeof(float) * 3);
    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(0, 0);

    glBindVertexBuffer(1, vbo[1], 0, sizeof(float) * 3);
    glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(1, 1);

    glBindVertexBuffer(2, vbo[2], 0, sizeof(float) * 2);
    glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(2, 2);
#endif

    glBindVertexArray(0);
}

void SceneBasic_Uniform::buildGround()
{
    static const float groundPos[] = {
        -10.0f, -0.5f, -10.0f,   10.0f, -0.5f, -10.0f,   10.0f, -0.5f,  10.0f,
        -10.0f, -0.5f, -10.0f,   10.0f, -0.5f,  10.0f,  -10.0f, -0.5f,  10.0f
    };

    static const float groundNorm[] = {
        0,1,0, 0,1,0, 0,1,0,
        0,1,0, 0,1,0, 0,1,0
    };

    static const float groundUV[] = {
        0,0, 10,0, 10,10,
        0,0, 10,10, 0,10
    };

    GLuint vbo[3];
    glGenBuffers(3, vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundPos), groundPos, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundNorm), groundNorm, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundUV), groundUV, GL_STATIC_DRAW);

    glGenVertexArrays(1, &groundVao);
    glBindVertexArray(groundVao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

#ifdef __APPLE__
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
#else
    glBindVertexBuffer(0, vbo[0], 0, sizeof(float) * 3);
    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(0, 0);

    glBindVertexBuffer(1, vbo[1], 0, sizeof(float) * 3);
    glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(1, 1);

    glBindVertexBuffer(2, vbo[2], 0, sizeof(float) * 2);
    glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(2, 2);
#endif

    glBindVertexArray(0);
}

void SceneBasic_Uniform::update(float t)
{
    float dt = t - lastTime;
    lastTime = t;

    if (!window) return;

    // Mouse look
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float xoffset = float(x - lastX);
    float yoffset = float(lastY - y);
    lastX = x;
    lastY = y;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camFront = glm::normalize(front);

    // Movement
    float vel = moveSpeed * dt;
    glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos += camFront * vel;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos -= camFront * vel;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos -= right * vel;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos += right * vel;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camPos -= camUp * vel;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camPos += camUp * vel;

    view = glm::lookAt(camPos, camPos + camFront, camUp);

    // Toggle dark/bright with F
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!togglePressed) {
            isDarkMode = !isDarkMode;
            togglePressed = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        togglePressed = false;
    }
}

void SceneBasic_Uniform::render()
{
    if (isDarkMode) {
        glClearColor(0.03f, 0.03f, 0.05f, 1.0f); // dark sky
    }
    else {
        glClearColor(0.62f, 0.70f, 0.85f, 1.0f); // bright sky
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    prog.use();

    // Lighting uniforms
    if (isDarkMode) {
        prog.setUniform("uLightColor", glm::vec3(0.9f, 0.9f, 1.0f));
        prog.setUniform("uAmbientStrength", 0.06f);
        prog.setUniform("uSpecStrength", 0.75f);
    }
    else {
        prog.setUniform("uLightColor", glm::vec3(2.5f, 2.5f, 2.5f));
        prog.setUniform("uAmbientStrength", 0.30f);
        prog.setUniform("uSpecStrength", 0.65f);
    }

    prog.setUniform("uShininess", 64.0f);

    // View/proj uniforms
    prog.setUniform("uView", view);
    prog.setUniform("uProj", projection);

    // Ground uses texture
    prog.setUniform("uUseTexture", 1);
    prog.setUniform("uTex", 0);

    glBindTextureUnit(0, floorTex);

    // Draw ground
    glm::mat4 groundModel(1.0f);
    prog.setUniform("uModel", groundModel);
    prog.setUniform("uBaseColor", glm::vec3(0.28f, 0.30f, 0.28f));

    glBindVertexArray(groundVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

	// Cube texture
    prog.setUniform("uUseTexture", 1);
    prog.setUniform("uTex", 0);
    glBindTextureUnit(0, cubeTex);

    // Draw cube 
    glm::mat4 cubeModel(1.0f);
    cubeModel = glm::translate(cubeModel, glm::vec3(0.0f, 0.0f, 0.0f));
    prog.setUniform("uModel", cubeModel);
    prog.setUniform("uBaseColor", glm::vec3(0.80f, 0.80f, 0.86f));

    glBindVertexArray(cubeVao);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);
}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);

    projection = glm::perspective(glm::radians(60.0f), float(w) / float(h), 0.1f, 200.0f);
}
