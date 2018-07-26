#version 450 core

layout (location = 0) in vec3 _inPos;

void main()
{
	gl_Position = vec4(_inPos.xy, 0.0, 1.0);
}