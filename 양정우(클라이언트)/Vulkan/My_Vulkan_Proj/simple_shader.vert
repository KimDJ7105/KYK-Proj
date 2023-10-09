#version 450			//glsl의 버전

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

layout (location = 0) out vec3 fragColor;

layout (push_constant) uniform Push {			// 푸시상수 데이터, 이름은 크게 신경 안써도 됨
	mat4 transform;
	vec3 color;
} push;

void main(){
	//gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);		// x, y, z, w
	gl_Position = push.transform * vec4(position, 1.0);
	fragColor = color;
}