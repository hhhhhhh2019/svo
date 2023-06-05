#version 460 core
precision highp float;

out vec3 color;
in flat int axis;
in vec3 vertex;

layout(location = 0, rgba32f) uniform image3D vox_pos;
//layout(binding = 0, offset = 0) uniform atomic_uint vox_count;

layout(location = 5) uniform float resolution;

void main() {
	color = vertex;

	vec3 pos = (vertex*0.5+0.5)*resolution;

	if (axis == 1)
		pos = pos.zyx;

	if (axis == 2)
		pos = pos.xzy;

	if (axis == 3)
		pos = pos.xyz;

	imageStore(vox_pos, ivec3(pos), vec4(1));
}
