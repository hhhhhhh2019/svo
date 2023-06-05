#include <model.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


void free_model(Model model) {
	free(model.data);
}


Model load_model_from_file(char* filename) {
	Model model = {0,NULL,vec3f(0,0,0),vec3f(0,0,0)};

	int f = open(filename, O_RDONLY);

	if (!f) {
		perror("open");
		exit(errno);
	}

	read(f, &model.resolution, sizeof(model.resolution));

	model.data = malloc(model.resolution*model.resolution*model.resolution*4*4);

	read(f, model.data, model.resolution*model.resolution*model.resolution*4*4);

	close(f);

	return model;
}


void save_model_to_file(Model model, char* filename) {

}
