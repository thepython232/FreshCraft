#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D textureAtlas;

void main() {
	//outColor = vec4(color, 1.0);
	outColor = texture(textureAtlas, uv);
	if (outColor.a < 0.01)
		discard;
	else
		outColor.a = 1.0;
	outColor.xyz *= max(dot(normal, normalize(vec3(0.3, 0.6, -1.0))), 0.2);
}