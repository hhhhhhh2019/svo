#include <image.h>
#include <svo.h>
#include <vector.h>
#include <model.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>


RenderImage output;
SVO tree;


#define threads_count 8

typedef struct {
	int x, ix;
	int y, iy;
	int w, iw;
	int h, ih;
} tile_data;


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

	if (tmax < 0 || tmin > tmax)
		return 0;

	*t = tmin < 0 ? tmax : tmin;
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


void* render_tile(void* arg) {
	tile_data* data = arg;

	for (; data->w > 0; data->w--, data->x++) {
		data->h = data->ih;
		data->y = data->iy;

		for (; data->h > 0; data->h--, data->y++) {
			vec3f rd = {(float)data->x / (float)output.width * 2 - 1, (float)data->y / (float)output.height * 2 - 1, 1};
			rd = vnormalize(rd);

			float t;
			Voxel* v = voxel_intersect(tree.root, (vec3f){0,0,-129}, rd, tree.levels, &t);

			if (v == NULL)
				continue;

			set_pixel(output, data->x,data->y, v->color.x,v->color.y,v->color.z);
		}
	}
}



/*void cat_node(ONode* node) {
	char childs = 0;

	for (int i = 0; i < 8; i++)
		if (node->childs[i] != NULL)
			childs++;

	if (childs != 8)
		return;

	for (int i = 0; i < 8; i++)
		cat_node(node->childs[i]);

	node->data = node->childs[0]->data;
	node->childs[0]->data = NULL;

	for (int i = 0; i < 8; i++)
		free_node(node->childs[i]);
}*/



int main() {
	time_t start_time = time(NULL);

	srand(start_time);

	output = init_image(512,512);
	tree = load_svo_from_file("object.oct");
	/*tree = init_svo(6);

	for (int x = 0; x < 2<<(tree.levels-1); x++) {
		for (int y = 0; y < 2<<(tree.levels-1); y++) {
			for (int z = 0; z < 2<<(tree.levels-1); z++) {
				if ((x & y & z) != 0)
					continue;

				Voxel* vox = malloc(sizeof(Voxel));
				vox->color = (vec3f){
					rand()&255,
					rand()&255,
					rand()&255
				};

				char ids[16];
				coords_to_ids(x,y,z, ids);
				if (add_data(&tree, tree.levels, ids, vox) == 0)
					free(vox);
			}
		}
	}

	save_svo_to_file(tree, "object.oct");*/


	//cat_node(tree.root);


	tile_data tiles[threads_count];
	pthread_t threads[threads_count];

	for (int i = 0; i < threads_count; i++) {
		tiles[i].x = tiles[i].ix = output.width / threads_count * i;
		tiles[i].y = tiles[i].iy = 0;
		tiles[i].w = tiles[i].iw = output.width / threads_count;
		tiles[i].h = tiles[i].ih = output.height;
	}


	time_t init_time = time(NULL);
	printf("init fisnished: %d\n", init_time - start_time);


	for (int i = 0; i < threads_count; i++) {
		pthread_create(&threads[i], NULL, render_tile, &tiles[i]);
	}

	for (int i = 0; i < threads_count; i++)
		pthread_join(threads[i], NULL);


	time_t draw_time = time(NULL);
	printf("draw fisnished: %d\n", draw_time - init_time);


	save_output(output, "output.png");
	free_image(output);

	free_svo(tree);
}
