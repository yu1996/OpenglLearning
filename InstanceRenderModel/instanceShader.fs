#version 330 core
struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};

#define NR_POINT_LIGHTS 2

in vec3 _FragPosition;
in vec3 _Normal;
in vec2 _TexCoords;

out vec4 _outColor;

uniform vec3 uViewPos;
uniform Material material;

void main()
{
	_outColor = texture(material.texture_diffuse1, _TexCoords);
}