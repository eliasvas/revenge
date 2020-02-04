#include "Tools.h" 
#include "ShaderProgram.h"
#pragma once
struct font_map {
	glm::vec2 screen_pos = {200,200};
	i32 font_width, font_height;
	i32 glyphs_per_row;
    GLuint vao, vbo1,vbo2, ebo;
	GLuint texture;
};

void init_text2D(font_map* arr,const char* texture_path = "../Data/Various/font.png");
void render_text2D(font_map* arr, const char* text,i32 x,i32 y,i32 size, ShaderProgram& shader);
