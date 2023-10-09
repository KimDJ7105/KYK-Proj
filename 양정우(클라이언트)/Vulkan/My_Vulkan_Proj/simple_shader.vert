#version 450			//glsl�� ����

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

layout (location = 0) out vec3 fragColor;

layout (push_constant) uniform Push {			// Ǫ�û�� ������, �̸��� ũ�� �Ű� �Ƚᵵ ��
	mat4 transform;
	vec3 color;
} push;

void main(){
	//gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);		// x, y, z, w
	gl_Position = push.transform * vec4(position, 1.0);
	fragColor = color;
}