#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(set = 0, binding = 0) uniform ShadowUBO {
	mat4 lightTransform;
} ubo;

layout(push_constant) uniform ChunkPush {
	ivec2 chunkPos;
} push;

void main() {
	gl_Position = ubo.lightTransform * vec4(inPos + vec3(push.chunkPos.x - 0.5, 0.0, push.chunkPos.y - 0.5) * 16.0, 1.0);
}