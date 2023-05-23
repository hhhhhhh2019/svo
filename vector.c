#include <vector.h>
#include <math.h>


vec3f vsum(vec3f a, vec3f b) {
	return (vec3f){a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3f vsub(vec3f a, vec3f b) {
	return (vec3f){a.x - b.x, a.y - b.y, a.z - b.z};
}

vec3f vmul(vec3f a, vec3f b) {
	return (vec3f){a.x * b.x, a.y * b.y, a.z * b.z};
}

vec3f vmulf(vec3f a, float b) {
	return (vec3f){a.x * b, a.y * b, a.z * b};
}

vec3f vmulm(vec3f a, mat3 b) {
	return (vec3f){
		a.x * b.a + a.y * b.b + a.z * b.c,
		a.x * b.d + a.y * b.e + a.z * b.f,
		a.x * b.g + a.y * b.h + a.z * b.i
	};
}

vec3f vdiv(vec3f a, vec3f b) {
	return (vec3f){a.x / b.x, a.y / b.y, a.z / b.z};
}

vec3f vdivf(vec3f a, float b) {
	return (vec3f){a.x / b, a.y / b, a.z / b};
}


mat3 vmmul(mat3 a, mat3 b) {
	return (mat3){
		a.a * b.a + a.b * b.d + a.c * b.g,
		a.a * b.b + a.b * b.e + a.c * b.h,
		a.a * b.c + a.b * b.f + a.c * b.i,

		a.d * b.a + a.e * b.d + a.f * b.g,
		a.d * b.b + a.e * b.e + a.f * b.h,
		a.d * b.c + a.e * b.f + a.f * b.i,

		a.g * b.a + a.h * b.d + a.i * b.g,
		a.g * b.b + a.h * b.e + a.i * b.h,
		a.g * b.c + a.h * b.f + a.i * b.i,
	};
}


float Q_rsqrt(float number) {
	const float x2 = number * 0.5F;
	const float threehalfs = 1.5F;

	union {
		float f;
		unsigned int i;
	} conv = {number};
	conv.i = 0x5f3759df - ( conv.i >> 1 );
	conv.f *= threehalfs - x2 * conv.f * conv.f;
	return conv.f;
}


float vdot(vec3f a, vec3f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}


float vlength(vec3f v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3f vnormalize(vec3f v) {
	//return vdivf(v, vlength(v));
	return vmulf(v, Q_rsqrt(vdot(v,v)));
}


vec3f vmin3(vec3f a, vec3f b) {
	return (vec3f){fmin(a.x,b.x), fmin(a.y,b.y), fmin(a.z,b.z)};
}

vec3f vmax3(vec3f a, vec3f b) {
	return (vec3f){fmax(a.x,b.x), fmax(a.y,b.y), fmax(a.z,b.z)};
}
