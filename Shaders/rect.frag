#version 460

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D textures[];

layout(push_constant) uniform RectPush {
	mat4 transform;
	int texID;
	float uvScale;
	vec2 uvOffset;
} push;

void main() {
	outColor = texture(textures[push.texID], inUV);
}