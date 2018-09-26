#version 450 core

layout (location = 0) in vec3 _inPos;
uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec3 _TexCoord3D;
void main()
{
	_TexCoord3D = _inPos / 2.0f + 0.5f;
	//gl_Position = vec4(_inPos, 1.0);
	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(_inPos, 1.0);
}