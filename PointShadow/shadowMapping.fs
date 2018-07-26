#version 330 core
out vec4 outFragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D uDiffuseTexture;
uniform samplerCube uShadowMap;

uniform vec3 uLightPos;
uniform vec3 uViewPos;

uniform float uFarPlane;
uniform bool uShadows;

vec3 sampleOffset[20] = vec3[](
vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);
float ShadowCalculation(vec3 fragPos)
{
	float shadow = 0.0;
	int sample = 20;
	float diskRadius = 0.05;
    float bias = 0.15;
	vec3 fragToLight = fragPos - uLightPos;
    float currentDepth = length(fragToLight);
	for(int i = 0; i < sample; ++i)
	{
		float closestDepth = texture(uShadowMap, fragToLight + sampleOffset[i] * diskRadius).r;
		closestDepth *= uFarPlane;
		if(currentDepth - bias > closestDepth) shadow += 1.0;
	}
	shadow /= float(sample);
    //float closestDepth = texture(shadowMap, fragToLight).r;
    //closestDepth *= far_plane;
    //float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;
        
    return shadow;
}

void main()
{           
    vec3 color = texture(uDiffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    vec3 lightDir = normalize(uLightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(uViewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = uShadows ? ShadowCalculation(fs_in.FragPos) : 0.0;                      
    vec3 lighting = (ambient + (1.0 - shadow ) * (diffuse + specular)) * color;    
    
    outFragColor = vec4(lighting, 1.0);
}