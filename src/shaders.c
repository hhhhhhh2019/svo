#include <shaders.h>
#include <window.h>
#include <utils.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


int render_prog;
int comp_prog;


int create_shader(char* source, int type) {
	int shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		printf("error: shader compilation failed:\n%s\n", infoLog);
	}

	return shader;
}


int create_program(int count, unsigned int shaders[]) {
	unsigned int prog = glCreateProgram();
	for (int i = 0; i < count; i++)
		glAttachShader(prog, shaders[i]);
	glLinkProgram(prog);

	int success;
	char infoLog[512];
	glGetProgramiv(prog, GL_LINK_STATUS, &success);

	if (!success) {
		glGetProgramInfoLog(prog, 512, NULL, infoLog);
		printf("error: program linking failed:\n%s\n", infoLog);
	}

	return prog;
}


void init_shaders() {
	char* vert_source;
	char* frag_source;
	char* comp_source;

	int tmp;

	read_file("shader.vert", &vert_source, &tmp);
	read_file("shader.frag", &frag_source, &tmp);
	read_file("shader.comp", &comp_source, &tmp);

	int vert_shader = create_shader(vert_source, GL_VERTEX_SHADER);
	int frag_shader = create_shader(frag_source, GL_FRAGMENT_SHADER);
	int comp_shader = create_shader(comp_source, GL_COMPUTE_SHADER);

	render_prog = create_program(2, (unsigned int[]){vert_shader, frag_shader});
	comp_prog = create_program(1, (unsigned int[]){comp_shader});

	free(vert_source);
	free(frag_source);
	free(comp_source);
}
