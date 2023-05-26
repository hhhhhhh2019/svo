#include <model.h>
#include <svo.h>


Model create_empty_model(int levels) {
	return (Model){
		init_svo(levels),
		(vec3f){0,0,0},
		(vec3f){0,0,0}
	};
}


void free_model(Model model) {
	free_svo(model.tree);
}


Model load_model_from_file(char* filename) {
	return (Model){
		load_svo_from_file(filename),
		(vec3f){0,0,0},
		(vec3f){0,0,0}
	};
}
