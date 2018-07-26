#version 330 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;

out vec2 outTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

uniform bool uReverseNormals;

void main()
{
    vs_out.FragPos = vec3(uModelMatrix * vec4(inPos, 1.0));
	if(uReverseNormals)
		vs_out.Normal = transpose(inverse(mat3(uModelMatrix))) * (-1.0 * inNormal);
	else
		vs_out.Normal = transpose(inverse(mat3(uModelMatrix))) * inNormal;
    vs_out.TexCoords = inTexCoords;
    gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(inPos, 1.0);
}