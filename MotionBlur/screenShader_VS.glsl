#version 450 core

layout(location = 0) in vec3 _inPos;

out vec2 _TexCoord;

void main()
{
	_TexCoord = _inPos.xy / 2.0f + 0.5f;
	gl_Position = vec4(_inPos.xy, 0.0, 1.0);
}