#version 450 core
layout (location = 0) in vec3 _inPos;

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
layout(binding = 0) uniform uModelMatrixUBO
{
	mat4 ModelMatrix[100];
};
void main()
{
    gl_Position = uProjectionMatrix * uViewMatrix * ModelMatrix[gl_InstanceID] * vec4(_inPos, 1.0);
}