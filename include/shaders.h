#ifndef SHADERS_H
#define SHADERS_H


extern int render_prog;
extern int comp_prog;

int create_shader(char*, int);
int create_program(int, unsigned int[]);

void init_shaders();


#endif // SHADERS_H
