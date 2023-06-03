#version 460
precision highp float;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

out int axis;

layout(location = 5) uniform float resolution;

out vec3 vertex;

void main() {
	mat4 proj;

	vec3 normal = normalize(cross(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz - gl_in[2].gl_Position.xyz));

	float ax = abs(normal.x);
	float ay = abs(normal.y);
	float az = abs(normal.z);

	if (ax > ay && ax > az) {
		proj = mat4(
			0,0,1,0,
			0,1,0,0,
			1,0,0,0,
			0,0,0,1
		);
		axis = 1;
	} else if (ay > ax && ay > az) {
		proj = mat4(
			1,0,0,0,
			0,0,1,0,
			0,1,0,0,
			0,0,0,1
		);
		axis = 2;
	} else {
		proj = mat4(
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1
		);
		axis = 3;
	}


	float h_pixel = 2./resolution;

	vec3 a = (gl_in[0].gl_Position * proj).xyz;
	vec3 b = (gl_in[1].gl_Position * proj).xyz;
	vec3 c = (gl_in[2].gl_Position * proj).xyz;

	vec3 center = (a+b+c) / 3;

	a += normalize(a-center)*h_pixel;
	b += normalize(b-center)*h_pixel;
	c += normalize(c-center)*h_pixel;


	gl_Position = vec4(a.xy, 0, 1);
	vertex = a;
	EmitVertex();
	gl_Position = vec4(b.xy, 0, 1);
	vertex = b;
	EmitVertex();
	gl_Position = vec4(c.xy, 0, 1);
	vertex = c;
	EmitVertex();
	EndPrimitive();
}
