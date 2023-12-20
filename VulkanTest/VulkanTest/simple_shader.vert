#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalworld;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionViewMatrix;
	vec4 ambientLightColor;
	vec3 lightPosition;
	vec4 lightColor;
} ubo;

// only 2 mat4 can be used to garentee that all platforms can run!
// this can be changed when the engine is ran on a specific system where you know the limit of

layout(push_constant) uniform Push{
    mat4 modelMatrix; // projection * view * model
	mat4 normalMatrix;
} push;

void main() {
	vec4 positionWorld = push.modelMatrix * vec4(position , 1.0);
	gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(position, 1.0);

	fragNormalworld = normalize(mat3(push.normalMatrix) * normal);
	fragPosWorld = positionWorld.xyz;
	fragColor = color;

	
}