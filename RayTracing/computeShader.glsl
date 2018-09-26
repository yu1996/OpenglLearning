#version 430 core

struct Sphere
{
	vec4 Position;
	vec4 Albedo;
	float Radius;
	int Type;
};

struct Ray{
	vec3 Origin;
	vec3 Direction;
};

struct IntersectPoint{
	vec3 Position;
	vec3 Normal;
	double T;
};
//constant
uniform vec3 uBackgroundColor = vec3(0.7, 0.8, 1.0);
uniform float uAttuenation = 0.5;
uniform int uMaxTracingNum = 5;
int CurrentTracingNum = 0;

ivec2 SampleOffset[] = { ivec2(0, 0), ivec2(1, 1), ivec2(1, -1),
ivec2(-1, -1), ivec2(-1, 1), ivec2(1, 0),
ivec2(-1, 0), ivec2(0, -1), ivec2(0, 1),
ivec2(2, 2), ivec2(2, -2), ivec2(2, 0),
ivec2(-2, 0), ivec2(-2, -2), ivec2(-2, 2),
ivec2(0, -2), ivec2(0, 2) };

const int SampleOffsetSize = 17;
const int MaxTracingCount = 1;

uniform vec3 uLightDirection = vec3(0.2, -1.0, 0.0);
uniform vec3 uLightColor = vec3(0.8);
uniform float uAmbientIntensity = 0.2;
uniform float uShininess = 64.0f;
uniform float uDiffuseAttenuationFactor = 0.95f;
uniform float uSpecularAttenuation = 0.95f;
//uniform from CPU
uniform vec3 uCameraPosition;
uniform vec3 uCameraDirection;
uniform vec3 uCameraUp;
uniform vec3 uCameraRight;
uniform float uNearPlaneHalfHeight;
uniform float uNearPlaneHalfWidth;
uniform int uSphereSetSize;

layout(local_size_x = 32, local_size_y = 32)in;

layout(binding = 1, rgba8) uniform image2D uImage;
layout(std140, binding = 0) uniform SphereUBO {
	Sphere sphereSet[3];
};

bool hitSphere(Ray vRay, vec3 vSphereCenter, float vRadius, out IntersectPoint voIntersect);
vec3 PointAtRay(Ray vRay, float t);

void main() {
	Ray ray[SampleOffsetSize];
	vec3 color[SampleOffsetSize];
	for (int i = 0; i < SampleOffsetSize; i++) {
		ray[i].Origin = uCameraPosition;

		vec2 TexCoord = (vec2(gl_GlobalInvocationID.xy) + vec2(SampleOffset[i]) * 0.5) / vec2(gl_WorkGroupSize.xy) / vec2(gl_NumWorkGroups.xy);
		vec2 NDCCoord = TexCoord * 2.0f - 1.0f;
		vec3 Direction = normalize(NDCCoord.x * uCameraRight * uNearPlaneHalfWidth + NDCCoord.y * uCameraUp * uNearPlaneHalfHeight + uCameraDirection * 0.01f);
		ray[i].Direction = Direction;

		color[i] = uBackgroundColor;
	}

	//TODO:ANTIALISING
	vec3 ResultColor = vec3(0.0);
	for (int n = 0; n < SampleOffsetSize; n++) {
		for (int i = 0; i < MaxTracingCount; ++i) {
			IntersectPoint closestIntersect;
			closestIntersect.T = -0.5f;
			closestIntersect.Position = vec3(1.0f, 0.0f, 0.0f);
			vec3 albedo = uBackgroundColor;
			for (int k = 0; k < uSphereSetSize; ++k) {
				IntersectPoint currentIntersect;
				if (hitSphere(ray[n], sphereSet[k].Position.xyz, sphereSet[k].Radius, currentIntersect)) {
					if (currentIntersect.T > 0 && (currentIntersect.T < closestIntersect.T || closestIntersect.T < 0.0f)) {
						closestIntersect = currentIntersect;
						albedo = sphereSet[k].Albedo.xyz;
						//Phong
						vec3 la = uAmbientIntensity * uLightColor * albedo;
						vec3 ld = dot(normalize(-uLightDirection), closestIntersect.Normal) * uLightColor * albedo;
						vec3 reflectDir = reflect(ray[n].Direction, closestIntersect.Normal);
						float specularAttenuation = pow(max(dot(-uLightDirection, reflectDir), 0.0), uShininess);
						vec3 ls = specularAttenuation * uLightColor * vec3(1.0);
						albedo = la + ld + ls;
					}
				}
			}
			if (closestIntersect.T < 0) {
				break;
			}
			ray[n].Origin = closestIntersect.Position;
			ray[n].Direction = reflect(ray[n].Direction, closestIntersect.Normal);
			
			Ray LightRay = Ray(closestIntersect.Position, -uLightDirection);
			IntersectPoint OcclusionPoint;
			float Shadow = 0.0f;
			for(int k = 0; k < uSphereSetSize; ++k)
				if (hitSphere(LightRay, sphereSet[k].Position.xyz, sphereSet[k].Radius, OcclusionPoint)) {
					Shadow = 0.98f;
					break;
				}
			color[n] = (1.0f - Shadow) * albedo;
		}
		ResultColor += color[n];
	}
	imageStore(uImage, ivec2(gl_GlobalInvocationID.xy), vec4(sqrt(ResultColor / float(SampleOffsetSize)), 0.0));

}

vec3 PointAtRay(Ray vRay, float t){
	return vRay.Origin + vRay.Direction * t;
}

bool hitSphere(Ray vRay, vec3 vSphereCenter, float vRadius, out IntersectPoint voIntersect){
	/*
	ray p(t) = Origin + t * Direction = A + t * B
	sphere (x - x0)^2 + (y - y0)^2 + (z - z0)^2 = Radius^2

	dot((p - c), (p - c)) = Radius^2
	dot((A + t * B - C),(A + t * B - C)) = R^2
	*/
	voIntersect.T = -1.0f;
	voIntersect.Position = vec3(10000.0f, 10000.0f, 10000.0f);
	vec3 OriginToSphereCenter = vRay.Origin - vSphereCenter;
	
	//ray origin is on the sphere
	if (abs(length(OriginToSphereCenter) - vRadius) < 0.0001) return false;

	if (dot(vRay.Direction, -OriginToSphereCenter) < 0) return false;

	double a = dot(vRay.Direction, vRay.Direction);
	double b = 2.0f * dot(OriginToSphereCenter, vRay.Direction);
	double c = dot(OriginToSphereCenter, OriginToSphereCenter) - double(vRadius) * vRadius;
	double discriminant = b * b - a * c * 4.0;
	if(discriminant > 0){
		double t = ((-b - sqrt(discriminant)) / (2.0 * a));
		if(t < 0){
			t = ((-b + sqrt(discriminant)) / (2.0 * a));
			if(t < 0) return false;
		}
		voIntersect.T = t;
		voIntersect.Position = PointAtRay(vRay, float(t));
		voIntersect.Normal = normalize(voIntersect.Position - vSphereCenter);
		return true;
	}
	return false;
}