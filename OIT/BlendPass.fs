#version 450 core

layout (binding = 0, r32ui) uniform uimage2D uStartOffsetImage;
layout (binding = 1, rgba32ui) uniform uimageBuffer uFragmentListBuffer;

layout (location = 0) out vec4 Color;

#define MAX_FRAGMENTS 15
uvec4 g_Fragments[MAX_FRAGMENTS];

uniform sampler2D uOpaqueColor;

in vec2 _TexCoord;

int buildLocalFragmentList(){
	uint Current = 0; 
	int FragCount = 0;

	Current = imageLoad(uStartOffsetImage, ivec2(gl_FragCoord.xy)).x;

	while(Current != 0 && FragCount < MAX_FRAGMENTS){
		uvec4 Fragment = imageLoad(uFragmentListBuffer, int(Current));
		Current = Fragment.x;
		g_Fragments[FragCount] = Fragment;
		FragCount++;
	}
	return FragCount;
}

void sortFragmentList(int vFragCount){
	for(int i = 0; i < vFragCount - 1; ++i){
		for(int j = i + 1; j < vFragCount - 1; ++j){
			if(uintBitsToFloat(g_Fragments[i].z) < uintBitsToFloat(g_Fragments[j].z)){
				uvec4 Temp = g_Fragments[j];
				g_Fragments[j] = g_Fragments[i];
				g_Fragments[i] = Temp;
			}
		}
	}
}
vec4 blend(vec4 vCurrentColor,  vec4 vNewColor){
	return vec4(vec3(vCurrentColor.xyz * (1.0 - vNewColor.w) + vNewColor.xyz * vNewColor.w), 1.0);
}
vec4 calculateFinalColor(int vFragCount){
	vec4 FinalColor = vec4(0.0);
	vec4 OpaqueColor = texture(uOpaqueColor, _TexCoord);

	if(vFragCount == 0) return OpaqueColor;

	vec4 LastColor = unpackUnorm4x8(g_Fragments[0].y);
	FinalColor = blend(OpaqueColor, LastColor);

	for(int i = 1; i < vFragCount; ++i){
		vec4 FragColor = unpackUnorm4x8(g_Fragments[i].y);
		FinalColor = blend(FinalColor, FragColor);
	}

	return FinalColor;
}

void main(){
	int FragCount = buildLocalFragmentList();

	sortFragmentList(FragCount);

	Color = calculateFinalColor(FragCount);
}