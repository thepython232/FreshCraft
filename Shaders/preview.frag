#version 450

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D tex;

void main() {
	float depth = texture(tex, inUV).r;
	outColor = vec4(depth, depth, depth, 1.0);
}