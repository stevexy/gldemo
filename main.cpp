
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
//#include <map>
//#include <stb_image.h>
//#include <learnopengl/filesystem.h>
//#include <string>

// 窗口尺寸
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 旋转角度初始值
float pitch = 0.0f;   // X轴
float yaw = 0.0f;     // Y轴
float roll = 0.0f;    // Z轴

// 着色器源码
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 ourColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
})";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;
void main()
{
    FragColor = vec4(ourColor, 1.0);
})";


void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 按键控制旋转
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) pitch -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) pitch += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) yaw -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) yaw += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) roll += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) roll -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) yaw = 90.0f;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        pitch = 0.0f;
        yaw = 0.0f;
        roll = 0.0f;
    }
}

int main(int argc, char* argv[]) {
    // 初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Gimbal Lock Demo", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // 编译着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // 加载PNG文件（需要stb_image.h）
    //int width, height, channels;
    //unsigned char* image = stbi_load(FileSystem::getPath("resources/textures/default8.png").c_str(), &width, &height, &channels, 0);
    //if (!image) {
    //    std::cerr << "Failed to load font texture!" << std::endl;
    //    return -1;
    //}

    //glGenTextures(1, &fontTextureID);
    //glBindTexture(GL_TEXTURE_2D, fontTextureID);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, image);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //stbi_image_free(image);


    // 顶点数据结构（位置+颜色）
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    // 立方体24个顶点（每个面4个顶点）
    Vertex vertices[] = {
        // 前表面（红色）
        { { 0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f} },  // 0
        { { 0.5f,-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f} },  // 1
        { {-0.5f,-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f} },  // 2
        { {-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f} },  // 3

        // 后表面（蓝色）
        { { 0.5f, 0.5f,-0.5f}, {0.0f, 0.0f, 1.0f} },  // 4
        { { 0.5f,-0.5f,-0.5f}, {0.0f, 0.0f, 1.0f} },  // 5
        { {-0.5f,-0.5f,-0.5f}, {0.0f, 0.0f, 1.0f} },  // 6
        { {-0.5f, 0.5f,-0.5f}, {0.0f, 0.0f, 1.0f} },  // 7

        // 右表面（绿色）
        { { 0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} },  // 8
        { { 0.5f,-0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} },  // 9
        { { 0.5f,-0.5f,-0.5f}, {0.0f, 1.0f, 0.0f} },  // 10
        { { 0.5f, 0.5f,-0.5f}, {0.0f, 1.0f, 0.0f} },  // 11

        // 左表面（青色）
        { {-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f} },  // 12
        { {-0.5f,-0.5f, 0.5f}, {0.0f, 1.0f, 1.0f} },  // 13
        { {-0.5f,-0.5f,-0.5f}, {0.0f, 1.0f, 1.0f} },  // 14
        { {-0.5f, 0.5f,-0.5f}, {0.0f, 1.0f, 1.0f} },  // 15

        // 上表面（黄色）
        { { 0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f} },  // 16
        { {-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f} },  // 17
        { {-0.5f, 0.5f,-0.5f}, {1.0f, 1.0f, 0.0f} },  // 18
        { { 0.5f, 0.5f,-0.5f}, {1.0f, 1.0f, 0.0f} },  // 19

        // 下表面（紫色）
        { { 0.5f,-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f} },  // 20
        { {-0.5f,-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f} },  // 21
        { {-0.5f,-0.5f,-0.5f}, {1.0f, 0.0f, 1.0f} },  // 22
        { { 0.5f,-0.5f,-0.5f}, {1.0f, 0.0f, 1.0f} }   // 23
    };

    // 索引数据（36个索引，每个面6个索引）
    unsigned int indices[] = {
        // 前表面
        0, 1, 2,  2, 3, 0,
        // 后表面
        4, 5, 6,  6, 7, 4,
        // 右表面
        8, 9,10, 10,11,8,
        // 左表面
        12,13,14, 14,15,12,
        // 上表面
        16,17,18, 18,19,16,
        // 下表面
        20,21,22, 22,23,20
    };

    // 配置VAO/VBO
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // 位置
    glEnableVertexAttribArray(0);
    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color)); // 颜色
    glEnableVertexAttribArray(1);

    // 投影矩阵
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);

    // 在gladLoadGL之后添加
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 标准透明混合

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 构建欧拉角旋转矩阵（XYZ顺序）
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));// X轴
        model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));  // Y轴
        model = glm::rotate(model, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f)); // Z轴

        // 传递矩阵到着色器
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // 渲染立方体
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // 显示当前角度
        std::cout << "\rPitch(X): " << pitch << "°  Yaw(Y): " << yaw << "°  Roll(Z): " << roll << "°  " << std::flush;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
    return 0;
}
