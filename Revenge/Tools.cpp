#include "Tools.h"
#include <iostream>
#include <fstream>
#include <algorithm>

namespace Tools
{
	char* LoadWholeStringFile(const char* filename)
	{
		// C++ code
		std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
		if (!in.is_open())
			return nullptr;

		size_t length = in.tellg();
		in.seekg(0, in.beg);

		char * buffer = new char[length + 1];
		in.read(buffer, length);
		buffer[length] = '\0';

		if (!in)
		{
			// error in reading the file
			delete[] buffer;
			buffer = nullptr;
		}
		in.close();

		return buffer;
	}

	std::string GetFolderPath(const char* filename)
	{
		std::string str(filename);
		size_t found;
		found = str.find_last_of("/\\");
		return str.substr(0, found + 1);
	}

	std::string tolowerCase(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		return str;
	}

	bool compareStringIgnoreCase(std::string str1, std::string str2)
	{
		str1 = tolowerCase(str1);
		str2 = tolowerCase(str2);
		return (str2.compare(str1) == 0);
	}

	GLenum CheckGLError()
	{
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			if (error == GL_INVALID_ENUM) printf("Error: Invalid Enum\n");
			else if (error == GL_INVALID_VALUE) printf("Error: Invalid Value\n");
			else if (error == GL_INVALID_OPERATION) printf("Error: Invalid Operation\n");
		}
		return error;
	}

	GLenum CheckFramebufferStatus(GLuint framebuffer_object)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);
		GLenum status;
		if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("glCheckFramebufferStatus: error %u\n", status);
			switch (status)
			{
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				printf("Incomplete attatchement\n");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				printf("Incomplete missing attachment\n");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				printf("Incomplete draw buffer\n");
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
				printf("Unsupported\n");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
				printf("Incomplete layer targets\n");
				break;
			default:
				printf("Default error\n");
				break;
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return status;
	}
};

bool in_bounds(int x, int y, int width, int height) {
	//std::cout << x << "   " << y << std::endl;
	return (x > 0 && x < width && y < height && y > 0);
	int r = x + y * width;
	return  (x > 0 && y > 0);
}

int count_ones(int* arr, int size) {
	int s = 0;
	for (int i = 0; i < size; ++i) {
		if (arr[i] == 1)++s;
	}
	return s;
}


//the path MUST be connected and not containing any circles 
std::vector<glm::vec2> find_path(int* arr, int width, int height) {
	std::vector<glm::vec2> positions;
	int k = 0;
	positions.push_back({0,0});
	int i = count_ones(arr, width*height);
	int y = 0;
	int x = 0;
	while (i--) {
		//std::cout << glm::to_string(positions.back()) << std::endl;
		//printf("%d\n", (int)in_bounds(k + 1, width, height));
		if (in_bounds(x+1,y, width, height) && positions.back() !=glm::vec2(x+1,y) && arr[x+1+y*width] == 1) {
			positions.push_back(glm::vec2(x+1,y));
			x++;
		}
		else if (in_bounds(x-1,y, width, height) && positions.back() !=glm::vec2(x-1,y) && arr[x-1+y*width] == 1) {
			positions.push_back(glm::vec2(x-1,y));
			x--;
		}
		else if (in_bounds(x,y+1, width, height) && positions.back() !=glm::vec2(x, y+1) && arr[x + (y+1)*width] == 1) {
			positions.push_back(glm::vec2(x, y+1));
			y++;
		}else if (in_bounds(x, y-1, width, height) && positions.back() !=glm::vec2(x, y-1) && arr[x+ (y-1)*width] == 1) {
			positions.push_back(glm::vec2(x,y-1));
			y--;
		}


	}
	return positions;
}
void print_vec2_arr(const std::vector<glm::vec2>& arr) {
	for (glm::vec2 v : arr)std::cout << glm::to_string(v) << std::endl;
}