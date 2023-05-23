#ifndef OCTREE_H
#define OCTREE_H


typedef struct {
	void* childs[8];
	void* value;
} ONode;


typedef struct {
	unsigned int levels;
	ONode* root;
} OcTree;


OcTree init_octree(int);
void free_octree(OcTree);

ONode* get_node_by_id(OcTree, int);
unsigned int get_id_by_xyz(OcTree, int, int, int);


#endif // OCTREE_H
