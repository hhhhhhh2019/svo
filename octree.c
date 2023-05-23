#include <octree.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


ONode* init_node(int);

ONode* init_node(int levels) {
	if (levels == 0)
		return NULL;

	ONode* node = malloc(sizeof(ONode));
	node->value = NULL;

	for (int i = 0; i < 8; i++)
		node->childs[i] = init_node(levels - 1);

	return node;
}


void free_node(ONode*);

void free_node(ONode* node) {
	if (node == NULL)
		return;

	for (int i = 0; i < 8; i++)
		free_node(node->childs[i]);

	if (node->value != NULL)
		free(node->value);

	free(node);
}


OcTree init_octree(int levels) {
	if (levels == 0)
		return (OcTree){0,NULL};

	ONode* root = init_node(levels);

	return (OcTree){
		levels,
		root
	};
}


void free_octree(OcTree tree) {
	free_node(tree.root);
}


ONode* get_node_by_id(OcTree tree, int id) {
	if (id >= pow(8,tree.levels-1)) {
		printf("\033[40mError (get_node_by_id):\033[39m id >= max tree id\n");
		return NULL;
	}

	ONode* node = tree.root;

	for (int i = 0; i < tree.levels-1; i++) {
		int m = pow(8,tree.levels-i-2);//(1 << (tree.levels - i - 1)) * 8;
		int lid = id / m;
		node = node->childs[lid];
		id -= lid * m;
	}

	return node;
}


unsigned int get_id_by_xyz(OcTree tree, int x, int y, int z) {
	int id = 0;

	for (int i = tree.levels - 2; i >= 0; i--) {
		int x_bid = x / (1 << i);
		int y_bid = y / (1 << i);
		int z_bid = z / (1 << i);

		x -= x_bid * (1 << i);
		y -= y_bid * (1 << i);
		z -= z_bid * (1 << i);

		int a = (x_bid + y_bid * 2 + z_bid * 4) * pow(8,i);

		printf("%d: %d %d %d: %d\n", i, x_bid, y_bid, z_bid, a);

		id += a;
	}

	return id;
}
