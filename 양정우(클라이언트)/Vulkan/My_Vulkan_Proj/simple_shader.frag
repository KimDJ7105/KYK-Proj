#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 0) out vec4 outColor;

layout (push_constant) uniform Push {			// 푸시상수 데이터, 이름은 크게 신경 안써도 됨
	mat4 transform;
	vec3 color;
} push;

void main(){
	outColor = vec4(fragColor, 1.0);
}