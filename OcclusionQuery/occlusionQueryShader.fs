#version 450 core

layout (binding = 0, rgba8) uniform imageBuffer uOcclusionQueryMap;

uniform sampler2D uModelIndexMap;

in vec2 _TexCoord;

//out vec4 _outFragColor;

void main(){
	float modelIndex = texture(uModelIndexMap, _TexCoord).x;
	imageStore(uOcclusionQueryMap, int(modelIndex), vec4(1, 0, 0, 1));
	//_outFragColor = vec4(modelIndex / (modelIndex + 2.0), 0.0, 0.0, 1.0);
}