#ifndef MODEL_H
#define MODEL_H


#include <octree.h>
#include <vector.h>



typedef struct {
	vec3f color;
} Voxel;


typedef struct {
	OcTree tree;
} Model;


Model create_empty_model(int);
Model load_model_from_file(char*);

void free_model(Model);


#endif // MODEL_H
