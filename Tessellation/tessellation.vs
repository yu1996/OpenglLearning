#version 430 core

layout (location = 0) in vec3 inPos;

uniform mat4 uModelMatrix;

void main()
{
	gl_Position = vec4(uModelMatrix * vec4(inPos, 1.0));
}