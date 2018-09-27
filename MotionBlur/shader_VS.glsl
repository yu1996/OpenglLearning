#version 430 core

layout(location = 0) in vec3 _inPosition;
layout(location = 1) in vec3 _inNormal;
layout(location = 2) in vec2 _inTexCoords;

out vec2 _TexCoords;
out vec3 _FragPosition;
out vec3 _Normal;

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat4 uModelMatrix;

void main() {
	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(_inPosition, 1.0);
	_FragPosition = vec3(uModelMatrix * vec4(_inPosition, 1.0));
	_Normal = mat3(transpose(inverse(uModelMatrix))) * _inNormal;
	_TexCoords = _inTexCoords;
}