#version 430 core
struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	float shininess;
};

layout(location = 0) out vec4 Color;
layout(location = 1) out vec4 Position;

in vec3 _FragPosition;
in vec3 _Normal;
in vec2 _TexCoords;

uniform vec3 uViewPos;
uniform Material material;

void main()
{
	Color = texture(material.texture_diffuse1, _TexCoords);
	Position = vec4(_FragPosition, gl_FragCoord.z);
}