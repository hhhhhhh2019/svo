#include <utils.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>


void read_file(char* filename, char** buffer, int* size) {
	int f = open(filename, O_RDONLY);

	if (f == -1) {
		perror("open");
		exit(errno);
	}

	struct stat st;
	fstat(f, &st);
	*size = st.st_size+1;

	*buffer = calloc(*size,1);

	read(f, *buffer, *size);

	close(f);
}
