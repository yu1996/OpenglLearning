#version 450 core

layout (location = 0) in vec3 _inPos;
uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec3 _TexCoord3D;
void main()
{
	_TexCoord3D = _inPos;
	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(_inPos, 1.0);
}