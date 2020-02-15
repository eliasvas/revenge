#version 330 core
layout(location = 0) out vec4 out_color;

uniform float alpha;

void main(void) 
{
	out_color = vec4( 0.2,1,0.1, alpha);	
}

