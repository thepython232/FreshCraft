#version 450

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 view;
	mat4 invView;
	mat4 proj;
	float fogNear, fogFar;
	vec4 fogColor;
} ubo;

layout(push_constant) uniform PushData {
	ivec3 blockPos;
} push;

vec3 corners[] = vec3[](
	vec3(0.0, 0.0, 0.0), //Front
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(1.0, 1.0, 0.0),
	vec3(1.0, 1.0, 0.0),
	vec3(1.0, 0.0, 0.0),
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 0.0, 0.0),
	
	vec3(0.0, 0.0, 1.0), //Back
	vec3(0.0, 1.0, 1.0),
	vec3(0.0, 1.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(1.0, 0.0, 1.0),
	vec3(1.0, 0.0, 1.0),
	vec3(0.0, 0.0, 1.0),
	
	vec3(0.0, 0.0, 0.0), //Bottom
	vec3(0.0, 0.0, 1.0),
	vec3(0.0, 0.0, 1.0),
	vec3(1.0, 0.0, 1.0),
	vec3(1.0, 0.0, 1.0),
	vec3(1.0, 0.0, 0.0),
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 0.0, 0.0),
	
	vec3(0.0, 1.0, 0.0), //Top
	vec3(0.0, 1.0, 1.0),
	vec3(0.0, 1.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(1.0, 1.0, 0.0),
	vec3(1.0, 1.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	
	vec3(1.0, 0.0, 0.0), //Right
	vec3(1.0, 0.0, 1.0),
	vec3(1.0, 0.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(1.0, 1.0, 0.0),
	vec3(1.0, 1.0, 0.0),
	vec3(1.0, 0.0, 0.0),
	
	vec3(0.0, 0.0, 0.0), //Left
	vec3(0.0, 0.0, 1.0),
	vec3(0.0, 0.0, 1.0),
	vec3(0.0, 1.0, 1.0),
	vec3(0.0, 1.0, 1.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 0.0)
);

void main() {
	gl_Position = ubo.proj * ubo.view * vec4(push.blockPos + corners[gl_VertexIndex], 1.0);
	//To avoid Z-Fighting
	gl_Position.z -= 0.001;
}