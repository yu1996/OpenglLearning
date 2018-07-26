#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aColor;
layout (location = 2) in vec2 aOffset;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec3 fColor;

void main()
{
    fColor = vec3(aColor, 0.5);
    gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPos + vec3(aOffset, 0.0), 1.0);
}