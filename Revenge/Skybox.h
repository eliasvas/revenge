#pragma once
#include <vector>
#include <string>
#include "ShaderProgram.h"
#include "SpotlightNode.h"
#include "ParticleSystem.h"
#include "GeometryNode.h"
#include "glm/glm.hpp"
#include "GLEW/glew.h"
#include "glm/gtc/type_ptr.hpp"
#include "CircleCollider.h"
#include "OBJLoader.h"
#include <iostream>
#include <vector>
#include "glm/gtx/string_cast.hpp"
#include "SDL2/SDL_timer.h"
#include "algorithm"

struct Skybox {
	void load_skybox(std::vector<std::string> faces) {
	
	}

	void render_skybox() {
	
	}
	int VAO;
	int VBO;
};
