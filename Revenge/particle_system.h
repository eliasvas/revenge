#pragma once
#include "Tools.h"
#include "GLEW\glew.h"
#include "glm\glm.hpp"
#include <vector>
#include "ShaderProgram.h"
#include "stb_image.h"


//NOTE abandon glm
struct particle_array {		
	glm::vec3 center; //default initialized to vec3(0)
	glm::vec3 p[10];
	glm::vec3 v[10];
	f32 l[10];
	float continuous_time;

    GLuint vao, vbo, ebo;
	GLuint texture;
};

bool Init_Particle(particle_array* arr);

void Render_Particle(particle_array* arr, ShaderProgram& shader);

void Update_Particle(particle_array* arr, f32 dt, f32 speed);

bool init_textured_particle(particle_array* arr,const char* texture_path);

void render_textured_particle(particle_array* arr, ShaderProgram& shader);

void update_textured_particle(particle_array* arr, f32 dt, f32 speed);


