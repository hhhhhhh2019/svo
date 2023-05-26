#version 460

out vec4 color;


uniform sampler2D tex;
layout(location = 0) uniform vec2 resolution;


void main() {
	vec2 uv = gl_FragCoord.xy / resolution;
	color = texture(tex, uv);
}
