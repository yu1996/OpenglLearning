#version 450 core

layout(location = 0) in vec3 _inPos;

out vec2 _TexCoord;

void main()
{
	gl_Position = vec4(_inPos.xy, 0.0, 1.0);
	_TexCoord = (_inPos.xy + 1.0) / 2.0f;
}