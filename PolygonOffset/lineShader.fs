#version 450 core

out vec4 _outFragColor;

uniform vec2 uRenderRegion;

void main()
{
	if(gl_FragCoord.x > uRenderRegion.y || gl_FragCoord.x < uRenderRegion.x) discard;
    _outFragColor = vec4(1.0, 0.0 ,0.0, 1.0);
}