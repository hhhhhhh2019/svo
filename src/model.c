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

	read(f, &model.data_size, sizeof(model.data_size));

	printf("%d\n", model.data_size);

	model.data = malloc(model.data_size);

	read(f, model.data, model.data_size);

	for (int i = 0; i < model.data_size/4; i++) {
		int a = model.data[i];
	}

	close(f);

	return model;
}


void save_model_to_file(Model model, char* filename) {
	int f = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0644);

	if (!f) {
		perror("open");
		exit(errno);
	}

	write(f, &model.data_size, sizeof(model.data_size));

	write(f, model.data, model.data_size);

	close(f);
}
