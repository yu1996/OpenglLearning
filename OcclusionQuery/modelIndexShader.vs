#version 450 core
layout (location = 0) in vec3 _inPos;
layout (location = 1) in vec2 _inTexCoords;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

void main()
{
    gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(_inPos, 1.0);
}