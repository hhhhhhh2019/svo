#ifndef VECTOR_H
#define VECTOR_H


typedef union {
	struct {float x,y,z;};
	float data[3];
} vec3f;

#define vec3f(vx,vy,vz) (vec3f){.x=vx,.y=vy,.z=vz}

typedef union {
	struct {float x,y;};
	float data[2];
} vec2f;

typedef struct {
	float a,b,c, d,e,f, g,h,i;
} mat3;


vec3f vsum(vec3f, vec3f);
vec3f vsub(vec3f, vec3f);
vec3f vmul(vec3f, vec3f);
vec3f vmulf(vec3f, float);
vec3f vmulm(vec3f, mat3);
vec3f vdiv(vec3f, vec3f);
vec3f vdivf(vec3f, float);

mat3 vmmul(mat3, mat3);

float vlength(vec3f);
vec3f vnormalize(vec3f);

vec3f vmin3(vec3f, vec3f);
vec3f vmax3(vec3f, vec3f);


#endif // VECTOR_H
