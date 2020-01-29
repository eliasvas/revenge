#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform float alpha;

void main()
{
    FragColor = texture(ourTexture, TexCoord);
	if(FragColor.a < 0.01)discard;
	FragColor.a = 1.f;
}
