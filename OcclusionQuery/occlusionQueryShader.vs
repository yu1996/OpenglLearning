#version 450 core

layout (location = 0) in vec3 _inPos;
layout (location = 1) in vec2 _inTexCoord;

out vec2 _TexCoord;

void main()
{
	gl_Position = vec4(_inPos.xy, 0.0, 1.0);
	_TexCoord = _inTexCoord;
}