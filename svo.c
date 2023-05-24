#include <svo.h>
#include <model.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>


SVO init_svo(char levels) {
	return (SVO){calloc(sizeof(ONode),1), levels};
}


void free_node(ONode* node) {
	if (node == NULL)
		return;

	for (int i = 0; i < 8; i++)
		free_node(node->childs[i]);

	if (node->data != NULL)
		free(node->data);

	free(node);
}


void free_svo(SVO tree) {
	free_node(tree.root);
	tree.levels = 0;
}


char add_data(SVO* tree, unsigned char level, char ids[], void* data) {
	if (level > MAX_LEVEL)
		level = MAX_LEVEL;

	ONode* root = tree->root;
	if (level > tree->levels)
		return 0;

	for (int i = level-1; i >= 0; i--) {
		if (root->childs[ids[i]] == NULL)
			root->childs[ids[i]] = calloc(sizeof(ONode),1);
		else if (root->childs[ids[i]]->data != NULL) {
			printf("error: try to add node to leaf\n");
			return 0;
		}

		if (root->data_dist == 0 || level-i+1 < root->data_dist)
			root->data_dist = level-i+1;
		root = root->childs[ids[i]];
	}

	if (root->data != NULL)
		return 0;

	root->data = data;
	root->data_dist = 1;

	return 1;
}


void* get_data(SVO* tree, unsigned char level, char ids[]) {
	ONode* root = tree->root;
	if (level > tree->levels)
		return NULL;

	for (int i = level-1; i >= 0; i--) {
		if (root->childs[ids[i]] == NULL)
			root->childs[ids[i]] = calloc(sizeof(ONode),1);

		root = root->childs[ids[i]];
	}

	return root->data;
}


void coords_to_ids(short x, short y, short z, char ids[]) {
	//printf("%d %d %d\n", x,y,z);
	for (int i = 0; i < MAX_LEVEL; i++) {
		ids[i] = (((z >> i) & 1) << 2) | (((y >> i) & 1) << 1) | ((x >> i) & 1);
		//printf("%d ", ids[i]);
	}
	//putc('\n', stdout);
}


void print_node(ONode* node, int level) {
	for (int i = 0; i < level; i++)
		putc('\t', stdout);

	printf("%d %p", level, node);

	if (node == NULL) {
		putc('\n', stdout);
		return;
	}

	printf(" %u %p\n", node->data_dist, node->data);

	for (int i = 0; i < 8; i++) {
		print_node(node->childs[i], level+1);
	}
}


void print_svo(SVO tree) {
	printf("%d\n", tree.levels);
	print_node(tree.root, 0);
}


void save_node_to_file(ONode* node, int f) {
	if (node == NULL)
		return;

	char childs = 0;

	for (int i = 0; i < 8; i++)
		if (node->childs[i] != NULL)
			childs |= 1 << i;

	write(f, &childs, 1);

	if (node->data != NULL) { // будем считать, что мы всегда храним тут воксели
		Voxel* v = node->data;

		write(f, &v->color, sizeof(v->color));
	} else {
		for (int i = 0; i < 8; i++)
			save_node_to_file(node->childs[i], f);
	}
}


void save_svo_to_file(SVO tree, char* filename) {
	int f = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0644);

	write(f, &tree.levels, sizeof(tree.levels));

	save_node_to_file(tree.root, f);

	close(f);
}


void load_node_from_file(ONode* node, int f) {
	char childs;

	read(f, &childs, 1);

	if (childs == 0) {
		vec3f color;
		read(f, &color, sizeof(color));

		Voxel* vox = malloc(sizeof(Voxel));
		vox->color = color;

		node->data = vox;
	} else {
		for (int i = 0; i < 8; i++) {
			if ((childs & (1 << i)) == 0)
				continue;

			node->childs[i] = calloc(sizeof(ONode),1);
			load_node_from_file(node->childs[i], f);
		}
	}
}



char count_data_dists(ONode* node) {
	if (node->data != NULL) {
		node->data_dist = 1;
		return 1;
	}

	for (int i = 0; i < 8; i++) {
		if (node->childs[i] == NULL)
			continue;

		char c = count_data_dists(node->childs[i]);

		if (node->data_dist < c + 1)
			node->data_dist = c;
	}

	return node->data_dist;
}


SVO load_svo_from_file(char* filename) {
	int f = open(filename, O_RDONLY);

	char levels;

	read(f, &levels, 1);

	SVO tree = init_svo(levels);

	load_node_from_file(tree.root, f);

	close(f);

	count_data_dists(tree.root);

	return tree;
}
