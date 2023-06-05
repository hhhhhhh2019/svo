#ifndef MODEL_H
#define MODEL_H


#include <vector.h>


typedef struct {
	unsigned int resolution;
	unsigned int* data;
	vec3f pos;
	vec3f rot;
} Model;


Model load_model_from_file(char*);
void save_model_to_file(Model,char*);

void free_model(Model);


#endif // MODEL_H
