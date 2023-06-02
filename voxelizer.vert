#version 460
precision highp float;

in vec3 position;

layout(location = 6) uniform float scale;

void main() {
	gl_Position = vec4(position*scale, 1);// * vec4(1,-1,1,1);
}
