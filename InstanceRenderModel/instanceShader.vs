#version 450 core

layout (location = 0) in vec3 _inPosition;
layout (location = 1) in vec3 _inNormal;
layout (location = 2) in vec2 _inTexCoords;
layout (location = 5) in mat4 _inInstanceModelMatrix;

out vec2 _TexCoords;
out vec3 _FragPosition;
out vec3 _Normal;

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

void main(){
	gl_Position = uProjectionMatrix * uViewMatrix * _inInstanceModelMatrix * vec4(_inPosition, 1.0);
	_FragPosition = vec3(_inInstanceModelMatrix * vec4(_inPosition, 1.0));
    _Normal = mat3(transpose(inverse(_inInstanceModelMatrix))) * _inNormal;
    _TexCoords = _inTexCoords;
}