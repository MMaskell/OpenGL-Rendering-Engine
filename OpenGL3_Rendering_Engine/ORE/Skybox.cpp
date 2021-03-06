#include "Skybox.h"
#include "Camera.h"
#include <stb_image.h>
namespace ORE {
	Skybox::Skybox() {
		cube = 0;
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), &vertexData, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
		program = getProgram("skybox");
		viewUniform = glGetUniformLocation(program, "view");
		projUniform = glGetUniformLocation(program, "projection");
		cubeSampler = glGetUniformLocation(program, "skybox");
		glBindVertexArray(0);
	}


	Skybox::~Skybox() {
		glDeleteVertexArrays(1, &vertexArray);
		glDeleteBuffers(1, &vertexBuffer);
	}


	void Skybox::draw(World* world) {
		//Only draw if the textures exist
		if (cube) {
			//Set shaders
			glUseProgram(program);
			glDepthFunc(GL_LEQUAL);
			glUniform1i(cubeSampler, 0);
			//Remove translation from camera
			glm::mat4 view = glm::mat4(glm::mat3(world->getCamera()->getView()));
			glm::mat4 proj = world->getCamera()->getProjection();
			glUniformMatrix4fv(viewUniform, 1, GL_FALSE, &view[0][0]);
			glUniformMatrix4fv(projUniform, 1, GL_FALSE, &proj[0][0]);
			glBindVertexArray(vertexArray);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cube);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glDepthFunc(GL_LESS);
		}
	}


	// Sets the textures of the skybox
	void Skybox::setTextures(const char* right, const char* left, const char* top, const char* bottom, const char* front, const char* back) {
		//Delete any old skybox
		if(cube) {
			glDeleteTextures(1, &cube);
			cube = 0;
		}
		glGenTextures(1, &cube);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cube);
		int width, height, channels;
		unsigned char* data = nullptr;
		//Right
		data = stbi_load(right, &width, &height, &channels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
		//Left
		data = stbi_load(left, &width, &height, &channels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
		//Top
		data = stbi_load(top, &width, &height, &channels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
		//Bottom
		data = stbi_load(bottom, &width, &height, &channels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
		//Front
		data = stbi_load(front, &width, &height, &channels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
		//Back
		data = stbi_load(back, &width, &height, &channels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
		//Make map seamless
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
}
