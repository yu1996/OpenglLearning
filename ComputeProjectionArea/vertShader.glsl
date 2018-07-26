#version 330 core
layout(location = 0) in vec3 _inPos;
layout(location = 2) in vec2 _inTexcoord;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec2 _Texcoord;

void main()
{
	_Texcoord = _inTexcoord;
	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(_inPos, 1.0);
}