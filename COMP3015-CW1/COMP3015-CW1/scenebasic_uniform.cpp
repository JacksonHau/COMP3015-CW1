#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <unordered_map>

#include "helper/glutils.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"

using glm::vec3;

SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f) {}

static GLuint loadTexture2D(const char* path)
{
    int w, h, n;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &n, 4);
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

    // Repeat 
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);
    return tex;
}

void SceneBasic_Uniform::compileUI()
{
    try {
        uiProg.compileShader("shader/ui_text.vert");
        uiProg.compileShader("shader/ui_text.frag");
        uiProg.link();
    }
    catch (GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::initUI()
{
    glGenVertexArrays(1, &uiVao);
    glGenBuffers(1, &uiVbo);

    glBindVertexArray(uiVao);
    glBindBuffer(GL_ARRAY_BUFFER, uiVbo);

    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

    glBindVertexArray(0);
}

void SceneBasic_Uniform::pushRect(float x, float y, float w, float h)
{
    float x0 = x, y0 = y;
    float x1 = x + w, y1 = y + h;

    uiRectVerts.insert(uiRectVerts.end(), { x0,y0,  x1,y0,  x1,y1 });
    uiRectVerts.insert(uiRectVerts.end(), { x0,y0,  x1,y1,  x0,y1 });
}

void SceneBasic_Uniform::pushText(float x, float y, const std::string& text)
{
    char tmp[20000];
    int quads = stb_easy_font_print(x, y, (char*)text.c_str(), nullptr, tmp, sizeof(tmp));

    float* v = (float*)tmp;
    for (int q = 0; q < quads; q++) {
        float* v0 = v + (q * 4 + 0) * 4;
        float* v1 = v + (q * 4 + 1) * 4;
        float* v2 = v + (q * 4 + 2) * 4;
        float* v3 = v + (q * 4 + 3) * 4;

        uiTextVerts.insert(uiTextVerts.end(), { v0[0], v0[1],  v1[0], v1[1],  v2[0], v2[1] });
        uiTextVerts.insert(uiTextVerts.end(), { v0[0], v0[1],  v2[0], v2[1],  v3[0], v3[1] });
    }
}

void SceneBasic_Uniform::drawOverlay()
{
    uiRectVerts.clear();
    uiTextVerts.clear();

    // UI text lines
    std::string l1 = "Controls:";
    std::string l2 = "Move - WASD";
    std::string l3 = "Look - Mouse";
    std::string l4 = "Up - Space";
    std::string l5 = "Down - Shift";
    std::string l6 = "Day/Night - L";
    std::string l7 = "Fog - F";
    std::string l8 = "Spotlight - Left Click";

    const int lineH = 18;
    const float pad = 10.0f;

	// Calculate panel size based on text width
    int w1 = stb_easy_font_width((char*)l1.c_str());
    int w2 = stb_easy_font_width((char*)l2.c_str());
    int w3 = stb_easy_font_width((char*)l3.c_str());
    int w4 = stb_easy_font_width((char*)l4.c_str());
    int w5 = stb_easy_font_width((char*)l5.c_str());
    int w6 = stb_easy_font_width((char*)l6.c_str());
    int w7 = stb_easy_font_width((char*)l7.c_str());
    int w8 = stb_easy_font_width((char*)l8.c_str());
    int maxW = std::max(std::max(std::max(std::max(w1, w2), std::max(w3, w4)), std::max(w5, w6)), std::max(w7, w8));

    float panelW = float(maxW) + pad * 2.0f;
    float panelH = float(lineH * 8) + pad * 2.0f;

    float x = float(width) - pad - panelW;
    float y = pad;

    // background panel
    pushRect(x, y, panelW, panelH);

    // text
    float tx = x + pad;
    float ty = y + pad;
    pushText(tx, ty + 0 * lineH, l1);
    pushText(tx, ty + 1 * lineH, l2);
    pushText(tx, ty + 2 * lineH, l3);
    pushText(tx, ty + 3 * lineH, l4);
    pushText(tx, ty + 4 * lineH, l5);
    pushText(tx, ty + 5 * lineH, l6);
    pushText(tx, ty + 6 * lineH, l7);
    pushText(tx, ty + 7 * lineH, l8);

    // Render overlay
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    uiProg.use();
    uiProg.setUniform("uScreen", glm::vec2((float)width, (float)height));

    glBindVertexArray(uiVao);
    glBindBuffer(GL_ARRAY_BUFFER, uiVbo);

    // draw panel
    uiProg.setUniform("uColor", glm::vec4(0.0f, 0.0f, 0.0f, 0.45f));
    glBufferData(GL_ARRAY_BUFFER, uiRectVerts.size() * sizeof(float), uiRectVerts.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(uiRectVerts.size() / 2));

    // draw text
    uiProg.setUniform("uColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    glBufferData(GL_ARRAY_BUFFER, uiTextVerts.size() * sizeof(float), uiTextVerts.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(uiTextVerts.size() / 2));

    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

struct GuardVert {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

static int FixIndex(int idx, int size) {
    if (idx > 0) return idx - 1;
    if (idx < 0) return size + idx;
    return -1;
}

static bool ParseVVTVN(const std::string& s, int& vi, int& ti, int& ni) {
    vi = ti = ni = 0;

    size_t p1 = s.find('/');
    if (p1 == std::string::npos) { 
        vi = std::stoi(s);
        return true;
    }
    size_t p2 = s.find('/', p1 + 1);
    if (p2 == std::string::npos) return false;

    vi = std::stoi(s.substr(0, p1));
    std::string t = s.substr(p1 + 1, p2 - (p1 + 1));
    std::string n = s.substr(p2 + 1);

    if (!t.empty()) ti = std::stoi(t);
    if (!n.empty()) ni = std::stoi(n);
    return true;
}

static bool LoadGuardOBJ_ByMaterial(
    const std::string& path,
    std::unordered_map<std::string, std::vector<GuardVert>>& outByMtl
) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ: " << path << "\n";
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texcoords;
    std::vector<glm::vec3> normals;

    std::string currentMtl = "default";

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            glm::vec3 v{};
            ss >> v.x >> v.y >> v.z;
            positions.push_back(v);
        }
        else if (type == "vt") {
            glm::vec2 t{};
            ss >> t.x >> t.y;
            t.y = 1.0f - t.y; 
            texcoords.push_back(t);
        }
        else if (type == "vn") {
            glm::vec3 n{};
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (type == "usemtl") {
            ss >> currentMtl;
            if (currentMtl.empty()) currentMtl = "default";
        }
        else if (type == "f") {
            std::vector<std::string> face;
            std::string tok;
            while (ss >> tok) face.push_back(tok);
            if (face.size() < 3) continue;

            auto emit = [&](const std::string& vtx) {
                int vi, ti, ni;
                if (!ParseVVTVN(vtx, vi, ti, ni)) return;

                int p = FixIndex(vi, (int)positions.size());
                int t = FixIndex(ti, (int)texcoords.size());
                int n = FixIndex(ni, (int)normals.size());

                if (p < 0 || p >= (int)positions.size()) return;

                GuardVert gv{};
                gv.pos = positions[p];

                gv.uv = (t >= 0 && t < (int)texcoords.size()) ? texcoords[t] : glm::vec2(0.0f);
                gv.normal = (n >= 0 && n < (int)normals.size()) ? normals[n] : glm::vec3(0, 1, 0);

                outByMtl[currentMtl].push_back(gv);
                };

            // triangulate 
            emit(face[0]); emit(face[1]); emit(face[2]);
            if (face.size() == 4) { emit(face[0]); emit(face[2]); emit(face[3]); }
        }
    }

    return true;
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

    compileUI();
    initUI();

    floorTex = loadTexture2D("assets/wood.png");
    cubeTex = loadTexture2D("assets/brick.jpg");

    // initial projection
    projection = glm::perspective(glm::radians(60.0f), float(width) / float(height), 0.1f, 200.0f);

    // GUARD: split OBJ by material
    std::unordered_map<std::string, glm::vec3> kd = {
        {"Botas",   glm::vec3(0.05f, 0.05f, 0.05f)},
        {"Chaleco", glm::vec3(0.05f, 0.05f, 0.05f)},
        {"Camisa",  glm::vec3(0.175139f, 0.383581f, 0.984887f)},
        {"Piel",    glm::vec3(0.940392f, 0.457111f, 0.267475f)},
        {"Vaquero", glm::vec3(0.026713f, 0.050565f, 0.119276f)}
    };

    std::unordered_map<std::string, std::vector<GuardVert>> byMtl;

    guardParts.clear();

    if (!LoadGuardOBJ_ByMaterial("assets/Guard.obj", byMtl)) {
        std::cerr << "Failed to load assets/Guard.obj\n";
        exit(1);
    }

    for (auto& kv : byMtl) {
        const std::string& mtlName = kv.first;
        auto& verts = kv.second;
        if (verts.empty()) continue;

        GuardPart part;
        part.count = (int)verts.size();

        auto it = kd.find(mtlName);
        part.kd = (it != kd.end()) ? it->second : glm::vec3(1.0f);

        glGenVertexArrays(1, &part.vao);
        glBindVertexArray(part.vao);

        glGenBuffers(1, &part.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, part.vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GuardVert), verts.data(), GL_STATIC_DRAW);

        // layout 0: position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GuardVert), (void*)offsetof(GuardVert, pos));

        // layout 1: normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GuardVert), (void*)offsetof(GuardVert, normal));

        // layout 2: uv 
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GuardVert), (void*)offsetof(GuardVert, uv));

        glBindVertexArray(0);

        guardParts.push_back(part);
    }

    std::cout << "Guard parts loaded: " << guardParts.size() << "\n";
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
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camPos -= camUp * vel;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camPos += camUp * vel;

    view = glm::lookAt(camPos, camPos + camFront, camUp);

    // Toggle dark/bright with L
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if (!togglePressed) {
            isDarkMode = !isDarkMode;
            togglePressed = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
        togglePressed = false;
    }

    // Toggle spotlight with T
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        if (!tPressed) {
            spotlightMode = !spotlightMode;
            tPressed = true;
        }
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE) {
        tPressed = false;
    }

    // Toggle FOG with F
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fPressed) {
            fogMode = !fogMode;
            fPressed = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        fPressed = false;
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

    prog.setUniform("uViewPos", camPos);
    prog.setUniform("uLightPos", lightPos);
    prog.setUniform("uLightColor", glm::vec3(1.2f, 1.0f, 0.85f));

    prog.setUniform("uShininess", 64.0f);

    // View/proj uniforms
    prog.setUniform("uView", view);
    prog.setUniform("uProj", projection);
    prog.setUniform("uFog", fogMode ? 1 : 0);

    prog.setUniform("uUseSpotlight", spotlightMode ? 1 : 0);

    // Make spotlight act like a flashlight from the camera
    if (spotlightMode) {
        prog.setUniform("uLightPos", camPos);
        prog.setUniform("uSpotDir", camFront);

        // inner/outer angles (degrees)
        float inner = glm::cos(glm::radians(12.5f));
        float outer = glm::cos(glm::radians(18.0f));
        prog.setUniform("uInnerCutoff", inner);
        prog.setUniform("uOuterCutoff", outer);
    }
    else {
        // Normal mode: keep your orbiting point light
        prog.setUniform("uLightPos", lightPos);

        // Still set something valid
        prog.setUniform("uSpotDir", glm::vec3(0.0f, -1.0f, 0.0f));
        prog.setUniform("uInnerCutoff", glm::cos(glm::radians(12.5f)));
        prog.setUniform("uOuterCutoff", glm::cos(glm::radians(18.0f)));
    }

    // Fog settings
    if (isDarkMode) {
        prog.setUniform("uFogColor", glm::vec3(0.05f, 0.05f, 0.08f));
    }
    else {
        prog.setUniform("uFogColor", glm::vec3(0.62f, 0.70f, 0.85f));
    }
    prog.setUniform("uFogNear", 6.0f);
    prog.setUniform("uFogFar", 25.0f);

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
    cubeModel = glm::translate(cubeModel, glm::vec3(3.0f, 0.0f, 0.0f));
    prog.setUniform("uModel", cubeModel);
    prog.setUniform("uBaseColor", glm::vec3(0.80f, 0.80f, 0.86f));

    glBindVertexArray(cubeVao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // Draw Guard first using the 3D shader
    prog.use(); 

    prog.setUniform("uUseTexture", 0);

    glm::mat4 guardModel(1.0f);
    guardModel = glm::translate(guardModel, glm::vec3(0.0f, 0.0f, 0.0f));
    guardModel = glm::scale(guardModel, glm::vec3(1.5f));
    prog.setUniform("uModel", guardModel);

    for (auto& part : guardParts) {
        prog.setUniform("uBaseColor", part.kd);
        glBindVertexArray(part.vao);
        glDrawArrays(GL_TRIANGLES, 0, part.count);
    }

    glBindVertexArray(0);
    glDrawArrays(GL_TRIANGLES, 0, guardVertexCount);
    glBindVertexArray(0);

    drawOverlay();
}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);

    projection = glm::perspective(glm::radians(60.0f), float(w) / float(h), 0.1f, 200.0f);
}
