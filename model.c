#include <model.h>
#include <octree.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


Model create_empty_model(int levels) {
	Model model;

	model.tree = init_octree(levels);

	return model;
}


Model load_model_from_file(char* filename) {
	int f = open(filename, O_RDONLY);

	if (!f) {
		perror("open");
		exit(errno);
	}

	int levels;
	read(f, &levels, 4);

	Model model = create_empty_model(levels);


	for (int i = 0; i < pow(8,levels-1); i++) {
		ONode* node = get_node_by_id(model.tree, i);

		char r,g,b,flags;

		read(f, &r, 1);
		read(f, &g, 1);
		read(f, &b, 1);
		read(f, &flags, 1);

		if (flags == 0)
			continue;

		Voxel* v = malloc(sizeof(Voxel));

		v->color.x = r;
		v->color.y = g;
		v->color.z = b;

		node->value = v;
	}


	close(f);

	return model;
}


void save_model_to_file(Model model, char* filename) {
	int f = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0644);

	if (!f) {
		perror("open");
		exit(errno);
	}

	write(f, &model.tree.levels, sizeof(model.tree.levels));

	for (int i = 0; i < pow(8,model.tree.levels-1); i++) {
		ONode* node = get_node_by_id(model.tree, i);

		char r = 0;
		char g = 0;
		char b = 0;
		char flags = 1;

		if (node->value == NULL) {
			flags = 0;
		} else {
			r = ((Voxel*)node->value)->color.x;
			g = ((Voxel*)node->value)->color.y;
			b = ((Voxel*)node->value)->color.z;
		}

		write(f, &r, 1);
		write(f, &g, 1);
		write(f, &b, 1);
		write(f, &flags, 1);
	}

	close(f);
}


void free_model(Model model) {
	free_octree(model.tree);
}
