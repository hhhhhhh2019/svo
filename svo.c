#include <svo.h>
#include <stdlib.h>
#include <stdio.h>


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
