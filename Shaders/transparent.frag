#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec4 inLightSpacePos;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D textureAtlas;

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 invView;
	mat4 proj;
	float fogNear, fogDist;
	vec4 fogColor;
	vec3 lightDir;
	vec4 lightColor;
} ubo;

void main() {
	outColor = texture(textureAtlas, inUV);
	vec3 light;
	light = vec3(0.2);
	light += ubo.lightColor.xyz * ubo.lightColor.w * max(dot(inNormal, -ubo.lightDir), 0.0);
	outColor.xyz *= light;

	float dist = (length(inPos - ubo.invView[3].xyz) - ubo.fogNear) / ubo.fogDist;
	dist = clamp(dist, 0.0, 1.0);
	outColor.xyz = (1.0 - dist) * outColor.xyz + dist * ubo.fogColor.xyz;
}