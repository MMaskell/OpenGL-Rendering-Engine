#include "Engine.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//Flag that we could like to use the dedicated graphics card if multiple are available
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

GLFWwindow* ORE::window;
std::map<std::string, GLuint> ORE::programs;
std::map<std::string, GLuint> ORE::textures;

void ORE::__sharedInit() {
	if (!window) {
		std::cerr << "Could not create window" << std::endl;
		glfwTerminate();
		exit(1);
	}
	//New OpenGL only
	glfwWindowHint(GLFW_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_VERSION_MINOR, 3);
	//No ignoring the above please
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwMakeContextCurrent(window); // Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Could not initialise GLEW" << std::endl;
		glfwTerminate();
		exit(1);
	}
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl
		<< "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
		<< "Vendor: " << glGetString(GL_VENDOR) << std::endl;
}


void ORE::init(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share) {
	if (!glfwInit()) {
		std::cerr << "Could not initialise glfw" << std::endl;
		exit(1);
	}
	window = glfwCreateWindow(width, height, title, monitor, share);
	__sharedInit();
}


void ORE::initFullscreen(const char* title, GLFWmonitor* monitor) {
	if (!glfwInit()) {
		std::cerr << "Could not initialise glfw" << std::endl;
		exit(1);
	}
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	window = glfwCreateWindow(mode->width, mode->height, title, monitor, NULL);
	__sharedInit();
}


void ORE::destroy() {
	if (window) {
		glfwDestroyWindow(window);
	}
	glfwTerminate();
}


void ORE::setVSync(bool vsync) {
	if (window) {
		glfwSwapInterval(vsync ? 1 : 0);
	}
}


GLuint ORE::getProgram(std::string program) {
	GLuint p = programs[program];
	if (p) {
		return p;
	}
	std::string vertString = readFile("ORE/shaders/" + program + ".vert");
	std::string fragString = readFile("ORE/shaders/" + program + ".frag");
	const char* vertChars = vertString.c_str();
	const char* fragChars = fragString.c_str();
	if (*vertChars && *fragChars) {
		GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
		//Compile vertex shader
		glShaderSource(vertex, 1, &vertChars, NULL);
		glCompileShader(vertex);
		//Check for errors
		GLint result = GL_FALSE;
		int logLength;
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &result);
		glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &logLength);
		if (result!=GL_TRUE || logLength > 0) {
			std::vector<char> errorMessage(logLength + 1);
			glGetShaderInfoLog(vertex, logLength, NULL, &errorMessage[0]);
			printf("ERROR: %s\n", &errorMessage[0]);
		}
		//Compile fragment shader
		glShaderSource(fragment, 1, &fragChars, NULL);
		glCompileShader(fragment);
		//Check for errors
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &result);
		glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &logLength);
		if (result != GL_TRUE || logLength > 0) {
			std::vector<char> errorMessage(logLength + 1);
			glGetShaderInfoLog(fragment, logLength, NULL, &errorMessage[0]);
			printf("ERROR: %s\n", &errorMessage[0]);
		}
		p = glCreateProgram();
		glAttachShader(p, vertex);
		glAttachShader(p, fragment);
		glLinkProgram(p);
		glGetProgramiv(p, GL_LINK_STATUS, &result);
		glGetProgramiv(p, GL_INFO_LOG_LENGTH, &logLength);
		if (result != GL_TRUE || logLength > 0) {
			std::vector<char> errorMessage(logLength + 1);
			glGetProgramInfoLog(p, logLength, NULL, &errorMessage[0]);
			printf("ERROR: %s\n", &errorMessage[0]);
		}
		glDetachShader(p, vertex);
		glDetachShader(p, fragment);
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		programs.insert_or_assign(program, p);
		return p;
	}
	std::cerr << "Couldn't loader shaders" << std::endl;
	return 0;
}


std::string ORE::readFile(std::string filename) {
	std::ifstream inStream(filename, std::ios::in);
	if (!inStream.is_open()) {
		return "";
	}
	std::string file;
	std::string line;
	while (std::getline(inStream, line))
		file += "\n" + line;
	inStream.close();
	return file;
}


GLuint ORE::loadImage(std::string filename) {
	//TODO: Handling to allow for automatic deletion
	if (textures[filename]) {
		return textures[filename];
	}
	GLuint tex;
	glGenTextures(1, &tex);
	GLenum err = glGetError();
	int width, height, channels;
	unsigned char* data = nullptr;
	data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
	if (data) {
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, channels == 3 ? GL_RGB : GL_RGBA, width, height, 0, channels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
		textures.insert_or_assign(filename, tex);
	}
	return tex;
}
