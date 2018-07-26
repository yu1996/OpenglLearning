#version 330 core
in vec4 FragPos;

uniform vec3 uLightPos;
uniform float uFarPlane;

void main()
{             
    float lightDistance = length(FragPos.xyz - uLightPos);

	lightDistance = lightDistance / uFarPlane;

	gl_FragDepth = lightDistance;
}