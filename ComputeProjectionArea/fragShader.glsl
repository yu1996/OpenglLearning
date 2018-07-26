#version 330 core
out vec4 FragColor;

in vec2 _Texcoord;
uniform sampler2D uTexture;

void main()
{
	FragColor = texture(uTexture, _Texcoord);
}