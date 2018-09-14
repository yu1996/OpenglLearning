#version 450 core

uniform sampler3D Noise;

uniform vec3 NoiseScale = vec3(4.0, 0.8, 0.8);//越大效果越明显

uniform vec3 Color1 = vec3(0.8, 0.7, 0.0);
uniform vec3 Color2 = vec3(0.6, 0.1, 0.0);

uniform vec3 SkyColor = vec3(0.0, 0.0, 0.8);
uniform vec3 CloudColor = vec3(0.8);

uniform vec3 VeinColor = vec3(0.1, 0.1, 0.05);
uniform vec3 MarbleColor = vec3(1.0);

uniform vec3 LightWood = vec3(0.6, 0.3, 0.1);
uniform vec3 DarkWood = vec3(0.4, 0.2, 0.07);
uniform float RingFreq = 4.0f;
uniform float LightGrains = 1.0;
uniform float DarkGrains = 0.0;
uniform float GrainThreshold = 0.5;
uniform float Noisiness = 3.0;
uniform float GrainsScale = 27.0f;

uniform float uLightIntensity = 1.0;

in vec3 _TexCoord3D;
out vec4 _outFragColor;

//sun surface
vec3 turbulence(vec4 vNoiseData, vec3 vColor1, vec3 vColor2){
	float intensity = abs(vNoiseData.x - 0.25) + abs(vNoiseData.y - 0.125) + abs(vNoiseData.z - 0.0625) + abs(vNoiseData.a - 0.03125);
	intensity = clamp(intensity * 6.0, 0.0, 1.0);
	return mix(vColor1, vColor2, intensity) * uLightIntensity;
}

//大理石
vec3 sinNoise(vec4 vNoiseData, vec3 vVeinColor, vec3 vMarbleColor){
	float intensity = abs(vNoiseData.x - 0.25) + abs(vNoiseData.y - 0.125) + abs(vNoiseData.z - 0.0625) + abs(vNoiseData.a - 0.03125);
	float sineval = sin(_TexCoord3D.y * 6.0 + intensity * 12.0) * 0.5 + 0.5;
	return mix(vVeinColor, vMarbleColor, intensity) * uLightIntensity;
}

//花岗岩
vec3 highFrequencyNoise(vec4 vNoiseData){
	float intensity = min(1.0, vNoiseData[3] * 18.0);
	return vec3(intensity * uLightIntensity);
}

//cloud
vec3 simpleNoise(vec4 vNoiseData, vec3 vSkyColor, vec3 vCloudColor){
	float intensity = (vNoiseData[0] + vNoiseData[1] + vNoiseData[2] + vNoiseData[3] + 0.03125) * 1.5;
	return mix(vSkyColor, vCloudColor, intensity) * uLightIntensity; 
}

//木纹
//看球不懂
vec3 wood(vec4 vNoiseData){
	vNoiseData.xyz += Noisiness;
	vec3 Location = _TexCoord3D + vNoiseData.xyz;
	float distance = sqrt(Location.x * Location.x + Location.z * Location.z);
	distance *= RingFreq;
	float r = fract(distance + vNoiseData[0] + vNoiseData[1] + vNoiseData[2]) * 2.0;

	if(r > 1.0) r = 2.0 - r;
	vec3 color = mix(LightWood, DarkWood, r);
	r = fract((_TexCoord3D.x + _TexCoord3D.z) * GrainsScale + 0.5);
	vNoiseData[2] *= r;
	if(r < GrainThreshold)
	color += LightWood * LightGrains * vNoiseData[2];
	else color += LightWood * DarkGrains * vNoiseData[2];
	color *= uLightIntensity;
	return color;
}

void main(){
	vec4 NoiseData = texture(Noise, _TexCoord3D * NoiseScale);
	//_outFragColor = vec4(turbulence(NoiseData, Color1, Color2), 1.0);
	//_outFragColor = vec4(simpleNoise(NoiseData, SkyColor, CloudColor), 1.0);
	//_outFragColor = vec4(sinNoise(NoiseData, VeinColor, MarbleColor), 1.0);
	//_outFragColor = vec4(highFrequencyNoise(NoiseData), 1.0);
	_outFragColor = vec4(wood(NoiseData), 1.0);
}