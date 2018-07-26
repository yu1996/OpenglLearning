#version 450 core
layout (location = 0) in vec3 _inPos;
layout (location = 1) in vec2 _inTexCoords;

out vec2 _TexCoords;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

void main()
{
    _TexCoords = _inTexCoords;
    gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(_inPos, 1.0);
}