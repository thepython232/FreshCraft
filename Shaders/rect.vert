#version 460

layout(location = 0) out vec2 outUV;

layout(push_constant) uniform RectPush {
	mat4 transform;
	int texID;
	float uvScale;
	vec2 uvOffset;
} push;

vec2 uvs[] = vec2[6](
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),
	vec2(1.0, 1.0),
	vec2(0.0, 1.0),
	vec2(0.0, 0.0)
);

void main() {
	outUV = uvs[gl_VertexIndex] * push.uvScale + push.uvOffset;
	gl_Position = push.transform * vec4(uvs[gl_VertexIndex] * 2.0 - 1.0, 0.0, 1.0);
	//gl_Position = vec4(0.3, 0.3, 0.0, 1.0) * vec4(outUV * 2.0 - 1.0, 0.0, 1.0);
}