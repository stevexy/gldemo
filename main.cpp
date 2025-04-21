
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>
#include <stb_image.h>
#include <learnopengl/filesystem.h>
#include <string>

// ���ڳߴ�
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ��ת�Ƕȳ�ʼֵ
float pitch = 0.0f;   // X��
float yaw = 0.0f;     // Y��
float roll = 0.0f;    // Z��

struct Character {
    GLuint     TextureID;  // ����������ID
    glm::ivec2 Size;       // ���δ�С
    glm::ivec2 Bearing;    // �ӻ�׼�ߵ�������/������ƫ��ֵ
    GLuint     Advance;    // ԭ�����һ������ԭ��ľ���
};

std::map<GLchar, Character> Characters;
unsigned int textVAO, textVBO;

// ��ɫ��Դ��
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

const char* textVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
})";


const char* textFragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
})";

void RenderText(unsigned int shaderProgram, std::string text, float x, float y, float scale, glm::vec3 color);

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // ����������ת
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
    // ��ʼ��GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // ��������
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Gimbal Lock Demo", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // ������ɫ��
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

	// �����ı���ɫ��
	unsigned int textVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(textVertexShader, 1, &textVertexShaderSource, NULL);
	glCompileShader(textVertexShader);
	unsigned int textFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(textFragmentShader, 1, &textFragmentShaderSource, NULL);
	glCompileShader(textFragmentShader);
	unsigned int textShaderProgram = glCreateProgram();
	glAttachShader(textShaderProgram, textVertexShader);
	glAttachShader(textShaderProgram, textFragmentShader);
	glLinkProgram(textShaderProgram);
	glDeleteShader(textVertexShader);
	glDeleteShader(textFragmentShader);
	// ��������
	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);	

    // ����PNG�ļ�����Ҫstb_image.h��
    int width, height, channels;
    unsigned char* image = stbi_load(FileSystem::getPath("resources/textures/default8.png").c_str(), &width, &height, &channels, 0);
    if (!image) {
        std::cerr << "Failed to load font texture!" << std::endl;
        return -1;
    }
    else {
        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
		unsigned int fontwidth = 8;
		unsigned int fontheight = 8;
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            // width 8  heighth 8
            // 256/16+128%16
            // 
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                fontwidth,
                fontheight,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                image
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(fontwidth, fontheight),
                glm::ivec2(0, 0),
                static_cast<unsigned int>(fontwidth)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(image);
    }

    //glGenTextures(1, &fontTextureID);
    //glBindTexture(GL_TEXTURE_2D, fontTextureID);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, image);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //


    // �������ݽṹ��λ��+��ɫ��
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    // ������24�����㣨ÿ����4�����㣩
    Vertex vertices[] = {
        // ǰ���棨��ɫ��
        { { 0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f} },  // 0
        { { 0.5f,-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f} },  // 1
        { {-0.5f,-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f} },  // 2
        { {-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f} },  // 3

        // ����棨��ɫ��
        { { 0.5f, 0.5f,-0.5f}, {0.0f, 0.0f, 1.0f} },  // 4
        { { 0.5f,-0.5f,-0.5f}, {0.0f, 0.0f, 1.0f} },  // 5
        { {-0.5f,-0.5f,-0.5f}, {0.0f, 0.0f, 1.0f} },  // 6
        { {-0.5f, 0.5f,-0.5f}, {0.0f, 0.0f, 1.0f} },  // 7

        // �ұ��棨��ɫ��
        { { 0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} },  // 8
        { { 0.5f,-0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} },  // 9
        { { 0.5f,-0.5f,-0.5f}, {0.0f, 1.0f, 0.0f} },  // 10
        { { 0.5f, 0.5f,-0.5f}, {0.0f, 1.0f, 0.0f} },  // 11

        // ����棨��ɫ��
        { {-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f} },  // 12
        { {-0.5f,-0.5f, 0.5f}, {0.0f, 1.0f, 1.0f} },  // 13
        { {-0.5f,-0.5f,-0.5f}, {0.0f, 1.0f, 1.0f} },  // 14
        { {-0.5f, 0.5f,-0.5f}, {0.0f, 1.0f, 1.0f} },  // 15

        // �ϱ��棨��ɫ��
        { { 0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f} },  // 16
        { {-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f} },  // 17
        { {-0.5f, 0.5f,-0.5f}, {1.0f, 1.0f, 0.0f} },  // 18
        { { 0.5f, 0.5f,-0.5f}, {1.0f, 1.0f, 0.0f} },  // 19

        // �±��棨��ɫ��
        { { 0.5f,-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f} },  // 20
        { {-0.5f,-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f} },  // 21
        { {-0.5f,-0.5f,-0.5f}, {1.0f, 0.0f, 1.0f} },  // 22
        { { 0.5f,-0.5f,-0.5f}, {1.0f, 0.0f, 1.0f} }   // 23
    };

    // �������ݣ�36��������ÿ����6��������
    unsigned int indices[] = {
        // ǰ����
        0, 1, 2,  2, 3, 0,
        // �����
        4, 5, 6,  6, 7, 4,
        // �ұ���
        8, 9,10, 10,11,8,
        // �����
        12,13,14, 14,15,12,
        // �ϱ���
        16,17,18, 18,19,16,
        // �±���
        20,21,22, 22,23,20
    };

    // ����VAO/VBO
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // λ������
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // λ��
    glEnableVertexAttribArray(0);
    // ��ɫ����
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color)); // ��ɫ
    glEnableVertexAttribArray(1);

    // ͶӰ����
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);

    // ��gladLoadGL֮������
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // ��׼͸�����

    // ��Ⱦѭ��
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ����ŷ������ת����XYZ˳��
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));// X��
        model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));  // Y��
        model = glm::rotate(model, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f)); // Z��

        // ���ݾ�����ɫ��
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // ��Ⱦ������
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        RenderText(textShaderProgram, "This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        // ��ʾ��ǰ�Ƕ�
        std::cout << "\rPitch(X): " << pitch << "��  Yaw(Y): " << yaw << "��  Roll(Z): " << roll << "��  " << std::flush;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
    return 0;
}

void RenderText(unsigned int shaderProgram, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
	glUseProgram(shaderProgram);
    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}