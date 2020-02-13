#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 uniform_view_matrix;
uniform mat4 uniform_projection_matrix;
uniform vec3 offset;
uniform vec3 scale;

void main()
{
	mat4 view = uniform_view_matrix;
	view[0][0] = -1.0;
	view[0][1] = 0.0;
	view[0][2] = 0.0;
	view[1][0] = 0.0;
	view[1][1] = 1.0;
	view[1][2] = 0.0;
	view[2][0] = 0.0;
	view[2][1] = 0.0;
	view[2][2] = 0.0;
	vec3 CameraRight_worldspace = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 CameraUp_worldspace = vec3(view[0][1], view[1][1], view[2][1]);
	vec4 position_wcs = vec4(aPos, 1.0);
	position_wcs.x = position_wcs.x * scale.x;
	position_wcs.y = position_wcs.y * scale.y;
	position_wcs.z = position_wcs.z * scale.z;
	position_wcs.x = position_wcs.x + offset.x;
	position_wcs.y = position_wcs.y + offset.y;
	position_wcs.z = position_wcs.z + offset.z;
    gl_Position = uniform_projection_matrix * view * position_wcs;
	
	
	
	
    TexCoord = aTexCoord;
}