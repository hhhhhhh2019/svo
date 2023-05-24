#include <image.h>
#include <svo.h>
#include <vector.h>
#include <model.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>


RenderImage output;
SVO tree;


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


Voxel* voxel_intersect(ONode* node, vec3f ro, vec3f rd, char level, float* t) {
	float size = (1 << level);

	if (!ray_box(ro,rd, size, t))
		return NULL;

	if (level == 0 || node->data != NULL)
		return node->data;

	Voxel* result = NULL;
	float tmin = INFINITY;

	for (int i = 0; i < 8; i++) {
		if (node->childs[i] == NULL)
			continue;

		float nt;

		float x = ((float)((i&1)>>0)-0.5) * size;
		float y = ((float)((i&2)>>1)-0.5) * size;
		float z = ((float)((i&4)>>2)-0.5) * size;

		Voxel* v = voxel_intersect(node->childs[i], vsum(ro, (vec3f){x,y,z}), rd, level-1, &nt);

		if (v != NULL && nt < tmin) {
			tmin = nt;
			result = v;
		}
	}

	*t = tmin;

	return result;
}


Voxel* render(vec3f ro, vec3f rd) {
	float t;
	return voxel_intersect(tree.root, ro, rd, tree.levels, &t);
}



int main() {
	time_t start_time = time(NULL);

	srand(start_time);

	output = init_image(2048,2048);
	tree = init_svo(6);

	for (int i = 0; i < 64; i++) {
		Voxel* vox = malloc(sizeof(Voxel));
		vox->color = (vec3f){
			rand()&255,
			rand()&255,
			rand()&255
		};

		short x = rand()&15;
		short y = rand()&15;
		short z = rand()&15;

		char ids[16];
		coords_to_ids(x,y,z, ids);
		if (add_data(&tree, (rand()&3)+3, ids, vox) == 0)
			free(vox);
	}

	time_t init_time = time(NULL);
	printf("init fisnished: %d\n", init_time - start_time);

	for (int i = 0; i < output.width; i++) {
		for (int j = 0; j < output.height; j++) {
			vec3f rd = {(float)i / output.width * 2 - 1, (float)j / output.height * 2 - 1, 1};
			rd = vnormalize(rd);

			Voxel* v = render((vec3f){0,0,-128}, rd);

			if (v != NULL)
				set_pixel(output, i,j, v->color.x,v->color.y,v->color.z);
		}
	}

	time_t draw_time = time(NULL);
	printf("draw fisnished: %d\n", draw_time - init_time);

	save_output(output, "output.png");
	free_image(output);

	free_svo(tree);
}
