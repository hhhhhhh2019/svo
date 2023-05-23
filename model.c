#include <model.h>
#include <octree.h>


Model create_empty_model(int levels) {
	Model model;

	model.tree = init_octree(levels);

	return model;
}


void free_model(Model model) {
	free_octree(model.tree);
}
