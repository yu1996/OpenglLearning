#version 450 core

layout (binding = 0, rgba8) uniform image2D uImage;

out vec4 _outFragColor;

void main(){
	_outFragColor = imageLoad(uImage, ivec2(gl_FragCoord.xy));
}