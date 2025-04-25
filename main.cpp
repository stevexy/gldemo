
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <map>
#include <stb_image.h>
#include <learnopengl/filesystem.h>
#include <string>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float pitch = 0.0f;   // X
float yaw = 0.0f;     // Y
float roll = 0.0f;    // Z

struct Character {
	GLuint     TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	GLuint     Advance;
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

void RenderCube(unsigned int cubeVAO, unsigned int shaderProgram,const glm::mat4 &projection,const glm::mat4 &view);
void RenderText(unsigned int shaderProgram, std::string text, float x, float y, float scale, glm::vec3 color,const glm::mat4 &projection,const glm::mat4 &view);
int BitmapFontGenerate();

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
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Gimbal Lock Demo", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

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
	
	// generate bitmap font
	BitmapFontGenerate();

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

	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
	};

	Vertex vertices[] = {
		{ { 0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f} },  // 0
		{ { 0.5f,-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f} },  // 1
		{ {-0.5f,-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f} },  // 2
		{ {-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f} },  // 3

		{ { 0.5f, 0.5f,-0.5f}, {0.0f, 0.0f, 1.0f} },  // 4
		{ { 0.5f,-0.5f,-0.5f}, {0.0f, 0.0f, 1.0f} },  // 5
		{ {-0.5f,-0.5f,-0.5f}, {0.0f, 0.0f, 1.0f} },  // 6
		{ {-0.5f, 0.5f,-0.5f}, {0.0f, 0.0f, 1.0f} },  // 7

		{ { 0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} },  // 8
		{ { 0.5f,-0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} },  // 9
		{ { 0.5f,-0.5f,-0.5f}, {0.0f, 1.0f, 0.0f} },  // 10
		{ { 0.5f, 0.5f,-0.5f}, {0.0f, 1.0f, 0.0f} },  // 11

		{ {-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f} },  // 12
		{ {-0.5f,-0.5f, 0.5f}, {0.0f, 1.0f, 1.0f} },  // 13
		{ {-0.5f,-0.5f,-0.5f}, {0.0f, 1.0f, 1.0f} },  // 14
		{ {-0.5f, 0.5f,-0.5f}, {0.0f, 1.0f, 1.0f} },  // 15

		{ { 0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f} },  // 16
		{ {-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f} },  // 17
		{ {-0.5f, 0.5f,-0.5f}, {1.0f, 1.0f, 0.0f} },  // 18
		{ { 0.5f, 0.5f,-0.5f}, {1.0f, 1.0f, 0.0f} },  // 19

		{ { 0.5f,-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f} },  // 20
		{ {-0.5f,-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f} },  // 21
		{ {-0.5f,-0.5f,-0.5f}, {1.0f, 0.0f, 1.0f} },  // 22
		{ { 0.5f,-0.5f,-0.5f}, {1.0f, 0.0f, 1.0f} }   // 23
	};

	unsigned int indices[] = {
		0 , 1, 2,  2, 3, 0,
		4 , 5, 6,  6, 7, 4,
		8 , 9,10, 10,11, 8,
		12,13,14, 14,15,12,
		16,17,18, 18,19,16,
		20,21,22, 22,23,20
	};

	unsigned int cubeVBO, cubeVAO, cubeEBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);

	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(1);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

	glm::mat4 otprojection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderCube(cubeVAO, shaderProgram,projection,view);
		std::stringstream ss;
		ss << "Pitch(X): " << pitch << "°  Yaw(Y): " << yaw << "°  Roll(Z): " << roll << "°  ";
		RenderText(textShaderProgram, ss.str(), 25.0f, 25.0f, 2.0f, glm::vec3(0.5, 0.8f, 0.2f), otprojection, view);
		
		RenderText(textShaderProgram, "Press up and down to change pitch, left & right for yaw", 25.0f, 68.0f, 1.5f, glm::vec3(0.8, 0.5f, 0.2f), otprojection, view);
		RenderText(textShaderProgram, "Q W for roll, R to reset, Y set the pitch to 90 degree", 25.0f, 50.0f, 1.5f, glm::vec3(0.8, 0.5f, 0.2f), otprojection, view);
		
//		std::cout << "\rPitch(X): " << pitch << "°  Yaw(Y): " << yaw << "°  Roll(Z): " << roll << "°  " << std::flush;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);
	glDeleteProgram(shaderProgram);
	glDeleteProgram(textShaderProgram);
	glDeleteVertexArrays(1, &textVAO);
	glDeleteBuffers(1, &textVBO);
	glfwTerminate();
	return 0;
}

int BitmapFontGenerate() {
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
		unsigned int glyphwidth = 128;
		unsigned char* chars = (unsigned char*)malloc(fontwidth * fontheight * 4);
		if (chars == nullptr) {
			std::cerr << "Error: 'chars' is null. Memory allocation failed." << std::endl;
			stbi_image_free(image);
			return -1;
		}

		for (unsigned char c = 0; c < 128; c++)
		{
			// Load character glyph
			// generate texture
			unsigned char* image_base = image + ((c / 16) * glyphwidth * 4 * 8) + ((c % 16) * fontwidth * 4);

			memset(chars, 0, fontwidth * fontheight * 4);
			
			// 查找左右边界并同时复制数据
			int left_edge = fontwidth;  // 默认为超出右侧边界
			int right_edge = -1;        // 默认为超出左侧边界
			
			for (int row = 0; row < fontheight; row++) {
				for (int col = 0; col < fontwidth; col++) {
					// 获取原始像素位置
					unsigned char* pixel = image_base + (row * glyphwidth * 4) + (col * 4);
					
					// 如果有不透明像素（alpha > 0）
					if (pixel[3] > 0) {
						// 更新边界
						if (col < left_edge) left_edge = col;
						if (col > right_edge) right_edge = col;
						
						// 直接复制有效像素到对应位置
						unsigned char* dst_pixel = chars + (row * fontwidth * 4) + (col * 4);
						dst_pixel[0] = pixel[0];
						dst_pixel[1] = pixel[1];
						dst_pixel[2] = pixel[2];
						dst_pixel[3] = pixel[3];
					}
				}
			}
			
			// 处理空字符的情况
			if (left_edge > right_edge) {
				left_edge = 0;
				right_edge = fontwidth - 1;
			}
			
			// 计算实际宽度
			int actual_width = right_edge - left_edge + 1;
			
			std::cout << "\r char width " << actual_width << " right " << right_edge << std::flush;
			
		 	unsigned int texture;
		 	glGenTextures(1, &texture);
		 	glBindTexture(GL_TEXTURE_2D, texture);
		 	glTexImage2D(
		 		GL_TEXTURE_2D,
		 		0,
		 		GL_RGBA,
		 		fontwidth,
		 		fontheight,
		 		0,
		 		GL_RGBA,
		 		GL_UNSIGNED_BYTE,
		 		chars
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
		 		glm::ivec2(0, fontheight),
		 		static_cast<unsigned int>((actual_width+1) * fontheight * 8 )
		 	};
		 	Characters.insert(std::pair<char, Character>(c, character));
		 }
		glBindTexture(GL_TEXTURE_2D, 0);

		// free image memory
		free(chars);
		stbi_image_free(image);
	}
}

void RenderCube(unsigned int cubeVAO, unsigned int shaderProgram,const glm::mat4 &projection,const glm::mat4 &view) {
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBindVertexArray(cubeVAO);
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));// X asix
	model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));  // Y
	model = glm::rotate(model, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f)); // Z

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

}

void RenderText(unsigned int shaderProgram, std::string text, float x, float y, float scale, glm::vec3 color,const glm::mat4 &projection,const glm::mat4 &view) {
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// activate corresponding render state
	glUseProgram(shaderProgram);
	
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
	
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
