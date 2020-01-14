#pragma once
#include <vector>
#include <string>
#include "ShaderProgram.h"
#include "glm/glm.hpp"
#include <iostream>
#include "Tools.h"
#include "GLEW/glew.h"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include "glm/gtx/string_cast.hpp"
#include "SDL2/SDL_timer.h"


u32 loadCubemap(std::vector<std::string> faces);
struct Skybox {
	Skybox(const std::vector<std::string>& faces) {
        load_skybox(faces);
		f32 skyboxVertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);	
    }

	void load_skybox(const std::vector<std::string>& faces) {
        this->faces = faces;
        tex_id = loadCubemap(faces);
		//make a cubemap_texture 
		//attach the images
		// render
	}

    void Render(ShaderProgram& shader) {
        //this render must be done at the beginning of the scene rendering process
        //because we disabled depth testing and basically only the color buffer gets updated
        glDepthMask(GL_FALSE);
		glBindVertexArray(VAO);
		glUniformMatrix4fv(shader["projection_matrix"], 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(shader["view_matrix"], 1, GL_FALSE, glm::value_ptr(view));

		glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);
		glDrawArrays(GL_TRIANGLES, 0,36);
        glDepthMask(GL_TRUE);
	}

	u32 VAO;
	u32 VBO;
    u32 tex_id;
    std::vector<std::string> faces;
    glm::mat4 projection;
    glm::mat4 view;
};
