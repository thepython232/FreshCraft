#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D textureAtlas;

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 invView;
	mat4 proj;
	float fogNear, fogDist;
	vec4 fogColor;
} ubo;

void main() {
	//outColor = vec4(color, 1.0);
	outColor = texture(textureAtlas, uv);
	outColor.xyz *= max(dot(normal, normalize(vec3(0.3, 0.6, -1.0))), 0.2);

	float dist = (length(pos - ubo.invView[3].xyz) - ubo.fogNear) / ubo.fogDist;
	dist = clamp(dist, 0.0, 1.0);
	outColor.xyz = (1.0 - dist) * outColor.xyz + dist * ubo.fogColor.xyz;
}