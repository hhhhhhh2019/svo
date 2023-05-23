#include <image.h>
#include <model.h>
#include <vector.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



RenderImage output;
Model obj;


char ray_box(vec3f ro, vec3f rd, float size, float *t) {
	float idx = 1. / rd.x;
	float idy = 1. / rd.y;
	float idz = 1. / rd.z;

	float t1 = (-size - ro.x) * idx;
	float t2 = ( size - ro.x) * idx;
	float t3 = (-size - ro.y) * idy;
	float t4 = ( size - ro.y) * idy;
	float t5 = (-size - ro.z) * idz;
	float t6 = ( size - ro.z) * idz;

	float tmin = fmax(fmax(fmin(t1,t2), fmin(t3,t4)), fmin(t5,t6));
	float tmax = fmin(fmin(fmax(t1,t2), fmax(t3,t4)), fmax(t5,t6));

	if (tmin < 0 || tmin > tmax)
		return 0;

	*t = tmin;
	return 1;
}


Voxel* voxel_intersect(vec3f ro, vec3f rd, ONode* node, int level, float *t) {
	float size = (1 << level) * 0.5;

	if (!ray_box(ro,rd, size, t))
		return NULL;

	if (level == 0) {
		return node->value;
	}

	Voxel* result = NULL;

	float tmin = INFINITY;

	for (int i = 0; i < 8; i++) {
		float x = ((float)(i % 2) - 0.5) * size;
		float y = ((float)(i / 2 % 2) - 0.5) * size;
		float z = ((float)(i / 4) - 0.5) * size;

		float nt;
		Voxel* v = voxel_intersect(vsub(ro, (vec3f){x,y,z}), rd, node->childs[i], level-1, &nt);

		if (v != NULL) {
			if (nt < tmin) {
				tmin = nt;
				result = v;
			}
		}
	}

	*t = tmin;

	return result;
}


int main() {
	time_t start_time = time(NULL);

	srand(start_time);

	output = init_image(1024,1024);
	obj = create_empty_model(8);

	//printf("%p\n", get_node_by_id(obj.tree, 8));

	for (int i = 0; i < pow(8,7); i++) {
		if (rand() % 128 != 0)
			continue;


		Voxel* vox = malloc(sizeof(Voxel));
		get_node_by_id(obj.tree, i)->value = vox;
		vox->color = (vec3f){
			(float)(rand()&255)/255.,
			(float)(rand()&255)/255.,
			(float)(rand()&255)/255.
		};
	}

	time_t draw_time = time(NULL);


	printf("init finish: %d\n", draw_time - start_time);


	for (int i = 0; i < output.width; i++) {
		for (int j = 0; j < output.height; j++) {
			vec3f rd = {(float)i / (float)output.width * 2 - 1, (float)j / (float)output.height * 2 - 1, 1};
			rd = vnormalize(rd);

			float t;
			Voxel* v = voxel_intersect((vec3f){0,0,-128}, rd, obj.tree.root, obj.tree.levels-1, &t);

			if (v == NULL)
				continue;

			set_pixel(output, i,j, v->color.x*255,v->color.y*255,v->color.z*255);
		}
	}

	printf("draw finish: %d\n", time(NULL) - draw_time);


	/*for (int i = 0; i < output.width; i++) {
		char r,g,b;

		get_pixel(output, i,output.height>>1, &r,&g,&b);
		set_pixel(output, i,output.height>>1, 255^r,255^g,255^b);

		get_pixel(output, output.width>>1,i, &r,&g,&b);
		set_pixel(output, output.width>>1,i, 255^r,255^g,255^b);
	}*/

	save_output(output, "output.png");
	free_image(output);
	free_model(obj);
}
