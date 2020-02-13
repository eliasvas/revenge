#include "text_rendering.h"
#include "stb_image.h"
#include "ShaderProgram.h"
#include "glm/gtc/type_ptr.hpp"

f32 f_positions_old[] = {
         0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f 
};
f32 f_positions[] = {
         0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f 
};
u32 f_indices[] = {  
        0, 1, 3, 
        1, 2, 3  
};
glm::vec2 uv_coords[4];

void init_text2D(font_map* arr,const char* texture_path) {
    glActiveTexture(GL_TEXTURE0);
    arr->font_width = 256;
    arr->font_height = 256;
    arr->glyphs_per_row = 16;
    glGenVertexArrays(1, &arr->vao);
	glGenBuffers(1, &arr->vbo1);
	glGenBuffers(1, &arr->vbo2);
	glGenBuffers(1, &arr->ebo);

	glBindVertexArray(arr->vao);

	glBindBuffer(GL_ARRAY_BUFFER, arr->vbo1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(f_positions), f_positions, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arr->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(f_indices), f_indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, arr->vbo2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uv_coords), &uv_coords[0], GL_DYNAMIC_DRAW);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void*)0);
	glEnableVertexAttribArray(1);


    glGenTextures(1, &arr->texture);
    glBindTexture(GL_TEXTURE_2D, arr->texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    //stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(texture_path, &width, &height, &nrChannels, 0);
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


}


void render_text2D(font_map* arr, const char* text, i32 x, i32 y, i32 size, ShaderProgram& shader) {
    //fill font_map with the right uv_coordinates via glSubBufferData (?) 
	for (int i = 0; i < 11; ++i) {
		int l = text[i];
		i32 w = (l) % arr->glyphs_per_row;
		i32 h = (l) / arr->glyphs_per_row;
		uv_coords[0] = glm::ivec2(w + 1, h + 1);
		uv_coords[1] = glm::ivec2(w + 1, h);
		uv_coords[2] = glm::ivec2(w, h);
		uv_coords[3] = glm::ivec2(w, h +1);

		for (int i = 0; i < 11; ++i) {
			uv_coords[i] /= arr->glyphs_per_row; //produce the text coordinates
		}

		//update vertex attributes ********************
		glBindVertexArray(arr->vao);
		glBindBuffer(GL_ARRAY_BUFFER, arr->vbo2);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 8 * sizeof(f32), &uv_coords[0]); //update uv_coordinate data 
		//--maybe syntax is wrong
		//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void*)0);
		//glEnableVertexAttribArray(1);

		glBindTexture(GL_TEXTURE_2D, arr->texture);
		glBindVertexArray(arr->vao);
		glUniform1f(shader["alpha"], 1.f);
		glUniform1i(shader["ourTexture"], 0);
		glUniform3f(shader["offset"], x- 0.3*i, y, 0);
		glUniform3f(shader["scale"], (-1.f)*0.3f, (-1.f)*0.3f, 1.f);
		//glUniformMatrix4fv(shader["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(projection_matrix)); --put ortho matrix (?)

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);
	}

}
