#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec4 inLightSpacePos;

layout(location = 0) out vec4 outColor;

void main() {
	//outColor = vec4(color, 1.0);
	outColor = vec4(inColor, 1.0);
}