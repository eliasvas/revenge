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
	vec4 position_wcs = vec4(aPos, 1.0);
	position_wcs.x = position_wcs.x * scale.x;
	position_wcs.y = position_wcs.y * scale.y;
	position_wcs.z = position_wcs.z * scale.z;
	position_wcs.x = position_wcs.x + offset.x;
	position_wcs.y = position_wcs.y + offset.y;
	position_wcs.z = position_wcs.z + offset.z;
    gl_Position = uniform_projection_matrix * uniform_view_matrix * position_wcs;
    TexCoord = aTexCoord;
}