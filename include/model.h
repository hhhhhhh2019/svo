#ifndef MODEL_H
#define MODEL_H


#include <vector.h>



typedef struct {
	vec3f color;
} Voxel;


typedef struct {
} Model;


Model create_empty_model(int);
Model load_model_from_file(char*);
void save_model_to_file(Model,char*);

void free_model(Model);


#endif // MODEL_H
