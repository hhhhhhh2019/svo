#ifndef MODEL_H
#define MODEL_H


#include <vector.h>
#include <svo.h>



typedef struct {
	char color[3];
} Voxel;


typedef struct {
	SVO tree;
	vec3f pos;
	vec3f rot;
} Model;


Model create_empty_model(int);
Model load_model_from_file(char*);
void save_model_to_file(Model,char*);

void free_model(Model);


#endif // MODEL_H
