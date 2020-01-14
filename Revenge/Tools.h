#pragma once
#include <string>
#include <iostream>
#include <vector>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>
#include "GLEW\glew.h"
#include <stdint.h>
typedef int32_t i32;
typedef uint32_t u32;
typedef float f32;

namespace Tools
{
	char* LoadWholeStringFile(const char* filename);

	std::string GetFolderPath(const char* filename);

	std::string tolowerCase(std::string str);

	bool compareStringIgnoreCase(std::string str1, std::string str2);

	GLenum CheckGLError();

	GLenum CheckFramebufferStatus(GLuint framebuffer_object);
};
bool in_bounds(int index, int width, int height);
int count_ones(int* arr, int size);
std::vector<glm::vec2> find_path(int* arr, int width, int height);
void print_vec2_arr(const std::vector<glm::vec2>& arr);