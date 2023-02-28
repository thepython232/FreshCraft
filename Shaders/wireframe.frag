#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	//outColor = vec4(color, 1.0);
	outColor = vec4(color, 1.0);
}