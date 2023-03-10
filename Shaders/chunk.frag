#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec4 inLightSpacePos;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D textureAtlas;

layout(set = 2, binding = 0) uniform sampler2D shadowMap;

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 invView;
	mat4 proj;
	float fogNear, fogDist;
	vec4 fogColor;
	vec3 lightDir;
	vec4 lightColor;
} ubo;

float Shadow(vec4 lightSpacePos) {
	//return 0.0;
	vec3 projectedCoords = lightSpacePos.xyz / lightSpacePos.w;

	projectedCoords.xy = projectedCoords.xy * 0.5 + 0.5;

	/*
	float closestDepth = texture(shadowMap, projectedCoords.xy).r;
	float currentDepth = projectedCoords.z;
	float bias = max(0.05 * (1.0 - dot(inNormal, -ubo.lightDir)), 0.005);
	bias = 0.001;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	*/

	float bias = 0.0002;
	float currentDepth = projectedCoords.z;

	if (currentDepth > 1.0 || currentDepth < 0.0) {
		return 0.0;
	}

	#define PCF_SAMPLES 5
	#define PCF_RANGE 0.9

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0) * PCF_RANGE;
	for (int x = -PCF_SAMPLES; x <= PCF_SAMPLES; x++) {
		for (int y = -PCF_SAMPLES; y < PCF_SAMPLES; y++) {
			float pcfDepth = texture(shadowMap, projectedCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow /= (PCF_SAMPLES * 2.0 + 1.0) * (PCF_SAMPLES * 2.0 + 1.0);

	return shadow;
}

void main() {
	//outColor = vec4(color, 1.0);
	outColor = texture(textureAtlas, inUV);
	outColor.xyz *= inColor;

	if (outColor.a < 0.01)
		discard;
	else
		outColor.a = 1.0;
	vec3 light = vec3(0.0);
	light += ubo.lightColor.xyz * ubo.lightColor.w * max(dot(inNormal, -ubo.lightDir), 0.0);

	//Shadow
	float shadow = Shadow(inLightSpacePos);

	outColor.xyz *= min((1.0 - shadow) * light + vec3(0.2), vec3(1.0));

	//Fog
	float dist = (length(inPos - ubo.invView[3].xyz) - ubo.fogNear) / ubo.fogDist;
	dist = clamp(dist, 0.0, 1.0);
	outColor.xyz = (1.0 - dist) * outColor.xyz + dist * ubo.fogColor.xyz;
}