#include "particle_system.h"
#include "stb_image.h"
#include <cstdlib>

f32 vertices[] = {
        // positions  // texture coords
         0.5f,  0.5f, 0.0f,1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,0.0f, 1.0f  // top left 
};
u32 indices[] = {  
        0, 1, 3, 
        1, 2, 3  
};

bool Init_Particle(particle_array* arr) {
	//arr->center = __;
	arr->continuous_time = 0.0f;
	for (int i = 0; i < 10; ++i) {
		arr->p[i] = arr->center;
		arr->v[i] = glm::vec3((float)rand()/(float)RAND_MAX,(float)rand()/(float)RAND_MAX+0.5f,(float)rand()/(float)RAND_MAX);
		arr->l[i] = 3.f * (float)rand()/(float)RAND_MAX;
	}
	glGenVertexArrays(1, &arr->vao);
	glBindVertexArray(arr->vao);

	glGenBuffers(1, &arr->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, arr->vbo);
	glBufferData(GL_ARRAY_BUFFER, 10 * sizeof(glm::vec3), &(arr->p[0]), GL_DYNAMIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		printf("Exiting with error at Renderer::Init\n");
		return false;
	}
	return true;
}

bool init_textured_particle(particle_array* arr, const char* texture_path) {
	glActiveTexture(GL_TEXTURE0);
	for (int i = 0; i < 10; ++i) {
		arr->p[i] = arr->center;
		arr->v[i] = glm::vec3((float)rand()/(float)RAND_MAX,(float)rand()/(float)RAND_MAX+0.5f,(float)rand()/(float)RAND_MAX);
		arr->l[i] = 3.f * (float)rand()/(float)RAND_MAX;
	}

	glGenVertexArrays(1, &arr->vao);
	glGenBuffers(1, &arr->vbo);
	glGenBuffers(1, &arr->ebo);

	glBindVertexArray(arr->vao);

	glBindBuffer(GL_ARRAY_BUFFER, arr->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arr->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(3 * sizeof(f32)));
	glEnableVertexAttribArray(1);


    glGenTextures(1, &arr->texture);
    glBindTexture(GL_TEXTURE_2D, arr->texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(texture_path, &width, &height, &nrChannels, 0);
	//for (int i = 0; i < 100; ++i)std::cout << (int)data[i] << std::endl;
	//maybe roll image
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("Failed to load texture\n");
    }
    stbi_image_free(data);
	
	return true;
}

void Render_Particle(particle_array* arr, ShaderProgram& shader) {
	glPointSize(10);
	glUniform1f(shader["alpha"], arr->continuous_time - (int)arr->continuous_time);
	glBindVertexArray(arr->vao);
	glDrawArrays(GL_POINTS, 0, (GLsizei)10);
}

void render_textured_particle(particle_array* arr, ShaderProgram& shader) {
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D,arr->texture);
	for (int i = 0; i < 10; ++i) {
		glUniform1f(shader["alpha"], arr->continuous_time - (int)arr->continuous_time);
		glUniform1i(shader["ourTexture"], 0);
		glUniform3f(shader["offset"], (f32)arr->p[i].x,arr->p[i].y,arr->p[i].z);
		glUniform3f(shader["scale"], 0.2f,0.2f,1.f);
		//glUniform3f(shader["offset"], i,2,2);
		//printf("vec3 = %d/%d/%d\n", arr->p[i].x,arr->p[i].y,arr->p[i].z);

		glBindTexture(GL_TEXTURE_2D,arr->texture);
		glBindVertexArray(arr->vao);
		glDrawElements(GL_TRIANGLES,6, GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);
	}
}

void Update_Particle(particle_array* arr,f32 dt,f32 speed) {
	for (int i = 0; i < 10; ++i) {
		if (arr->l[i] < 0.f) {
			arr->l[i] =  3.f * (float)rand()/(float)RAND_MAX;
			arr->p[i] = arr->center;
		}
		else {
			arr->p[i] += glm::vec3(7*cos(arr->continuous_time),arr->v[i].y*(4 - arr->l[i]),7*sin(arr->continuous_time))*dt*arr->v[i].x;
		}
		arr->l[i] -= dt;
	}
	//should update buffer data via glSubBufferData to pass into VRAM(?)
	glBindBuffer(GL_ARRAY_BUFFER, arr->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 10 * sizeof(glm::vec3), &(arr->p[0]));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	arr->continuous_time += dt;
}

void update_textured_particle(particle_array* arr, f32 dt, f32 speed) {
	for (int i = 0; i < 10; ++i) {
		if (arr->l[i] < 0.f) {
			arr->l[i] =  3.f * (float)rand()/(float)RAND_MAX;
			arr->p[i] = arr->center;
		}
		else {
			arr->p[i] += glm::vec3(7*cos(arr->continuous_time),arr->v[i].y*(4 - arr->l[i]),7*sin(arr->continuous_time))*dt*arr->v[i].x;
		}
		arr->l[i] -= dt;
	}
	arr->continuous_time += dt;
}

