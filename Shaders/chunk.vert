#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec2 outUv;
layout(location = 4) out vec4 outLightSpacePos;

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 invView;
	mat4 proj;
	float fogNear, fogDist;
	vec4 fogColor;
	vec3 lightDir;
	vec4 lightColor;
} ubo;

layout(set = 1, binding = 1) uniform ShadowUBO {
	mat4 lightTransform;
} shadowUBO;

layout(push_constant) uniform ChunkPushConstants {
	ivec2 pos;
} push;

void main() {
	//TODO: specialization constants
	outPos = pos + vec3(push.pos.x - 0.5, 0.0, push.pos.y - 0.5) * 16.0;
	gl_Position = ubo.proj * ubo.view * vec4(outPos, 1.0);
	outUv = uv;
	outColor = color;
	outNormal = normal;
	outLightSpacePos = shadowUBO.lightTransform * vec4(outPos, 1.0);
}