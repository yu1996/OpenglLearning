#version 450 core

layout (early_fragment_tests) in;
layout (binding = 0, r32ui) uniform uimage2D uStartOffsetImage;
layout (binding = 1, rgba32ui) uniform uimageBuffer uFragmentListBuffer;
layout (binding = 0, offset = 0) uniform atomic_uint uFragmentListCounter;
layout (location = 0) out vec4 ColorTex;

in vec2 _TexCoord;

uniform vec3 uViewPos;
uniform vec2 uScreenSize;
uniform sampler2D uDepthMap;
uniform sampler2D uWindowTexture;

void createFragmentLinkList(vec4 vFragmentColor, float vDepth)
{
	uint Index;
	uint OldStartOffset;
	uvec4 Fragment;
	Index = atomicCounterIncrement(uFragmentListCounter);
	OldStartOffset = imageAtomicExchange(uStartOffsetImage, ivec2(gl_FragCoord.xy), Index);
	Fragment.x = OldStartOffset;
	Fragment.y = packUnorm4x8(vFragmentColor);
	Fragment.z = floatBitsToUint(vDepth);
	Fragment.w = 0;

	imageStore(uFragmentListBuffer, int(Index), Fragment);
}

void main(){
	float OpaqueDepth = texture(uDepthMap, gl_FragCoord.xy / uScreenSize).x;
	if(gl_FragCoord.z > OpaqueDepth){
		discard;
	}
	else{
		float fSceneDepth = gl_FragCoord.z;
		vec4 DiffuseColor = texture2D(uWindowTexture, _TexCoord);

		createFragmentLinkList(DiffuseColor, fSceneDepth);
		ColorTex = DiffuseColor;
	}
}