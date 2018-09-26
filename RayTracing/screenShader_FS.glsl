#version 450 core

layout(binding = 0, rgba8) uniform image2D uImage;

uniform sampler2D uTexture;
uniform vec3 uScreenSize;
in vec2 _TexCoord;
out vec4 _outFragColor;

void main() {
	//vec2 TexelSize = 1.0f / uScreenSize;
	_outFragColor = texture(uTexture, _TexCoord);
}