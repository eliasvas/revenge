#version 330 core
layout(location = 0) in vec3 coord3d;

uniform mat4 uniform_view_matrix;
uniform mat4 uniform_projection_matrix;

void main(void) 
{
	mat4 view = uniform_view_matrix;
	//this is so there is no rotation for the quads
	view[0][0] = 1.0;
	view[0][1] = 0.0;
	view[0][2] = 0.0;
	view[1][0] = 0.0;
	view[1][1] = 1.0;
	view[1][2] = 0.0;
	view[2][0] = 0.0;
	view[2][1] = 0.0;
	view[2][2] = 0.0;
	gl_Position = uniform_projection_matrix * view * vec4(coord3d, 1.0);
}
