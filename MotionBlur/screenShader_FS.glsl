#version 430 core

uniform sampler2D uColor;
uniform sampler2D uPositionDepth;

uniform mat4 uPreviousVPMatrix;
uniform mat4 uCurrentVPMatrix;

const int uNumSample = 10;

in vec2 _TexCoord;
out vec4 _outFragColor;

void main() {
	float z = texture(uPositionDepth, _TexCoord).w;
	vec3 CurrentNDCCoord = vec3(_TexCoord * 2.0f - 1.0f, z);
	vec3 FragPosW = texture(uPositionDepth, _TexCoord).xyz;
	vec4 PreviousNDCCoord = uPreviousVPMatrix * vec4(FragPosW, 1.0f);

	vec3 PreviousCoord = PreviousNDCCoord.xyz / PreviousNDCCoord.w;

	vec2 velocity = (CurrentNDCCoord.xy - PreviousCoord.xy) / float(uNumSample);

	//有黑边，暂时这么解决
	vec4 Color = texture(uColor, _TexCoord) * 0.5;
	for(int i = 1; i < uNumSample; ++i){
		Color += texture(uColor, _TexCoord + float(i) * velocity) * 0.5 / (float(uNumSample) - 1);
	}
	if(gl_FragCoord.x > 400)
		_outFragColor = Color;
	else _outFragColor = texture(uColor, _TexCoord);

	if (z > 0.99) _outFragColor = texture(uColor, _TexCoord);

	if (gl_FragCoord.x > 400 && gl_FragCoord.x < 402) _outFragColor = vec4(1.0, 0.0, 0.0, 0.0);
}