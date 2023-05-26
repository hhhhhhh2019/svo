#ifndef RENDER_H
#define RENDER_H


#include <vector.h>
#include <model.h>


extern vec3f camera_pos;
extern vec3f camera_rot;
extern mat3 camera_rot_mat;


void init_render();
void set_uniforms();
void update_screen();
void update_camera_rot_mat();

void render_model(Model);


#endif
