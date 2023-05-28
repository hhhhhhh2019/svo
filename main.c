// функции шейдеров почему-то работают без glew, но при попытке его подключить некоторые фунции указывают на 0
// если эти фунции не работют, подключайте glew


#include <window.h>
#include <shaders.h>
#include <render.h>
#include <vector.h>
#include <model.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>


char running = 1;
xcb_generic_event_t* ev;


int mouse_vel_x = 0;
int mouse_vel_y = 0;

float move_speed = 10.;
float rot_speed = 0.01;

vec3f camera_vel = {0,0,0};

float delta = 0.;


void get_mouse_pos(int*,int*);
void update();
void draw();

void key_press(int);
void key_release(int);

long millis();


int main() {
	init_window(800,800);
	init_shaders();
	init_render();

	//srand(100);

	camera_pos = (vec3f){0,0,-32};

	//Model obj1 = load_model_from_file("object.oct");

	Model obj1 = create_empty_model(2);

	for (int x = 0; x < 2<<(obj1.tree.levels-1); x++) {
		for (int y = 0; y < 2<<(obj1.tree.levels-1); y++) {
			for (int z = 0; z < 2<<(obj1.tree.levels-1); z++) {
				char ids[16];
				coords_to_ids(x,y,z, ids);

				char skip = 0;

				for (int i = 0; i < 16; i++) {
					if (ids[i] == 7)
						skip = 1;
				}

				if (skip == 1)
					continue;

				Voxel* vox = malloc(sizeof(Voxel));
				vox->color[0] = rand()&255;
				vox->color[1] = rand()&255;
				vox->color[2] = rand()&255;

				if (add_data(&obj1.tree, obj1.tree.levels, ids, vox) == 0)
					free(vox);
			}
		}
	}

	//print_svo(obj1.tree);

	long last_time = millis();

	while (running) {
		xcb_warp_pointer(dpy, XCB_NONE, screen->root, 0,0,0,0, 100,100);

		render_model(obj1);


		while (ev = xcb_poll_for_event(dpy)) {
			switch (ev->response_type & ~0x80) {
				case XCB_CLIENT_MESSAGE:
					xcb_client_message_event_t* cm = (xcb_client_message_event_t*)ev;

					if (cm->data.data32[0] == delte_window_atom)
						running = 0;

					break;
				case XCB_KEY_PRESS:
					key_press(((xcb_key_press_event_t*)ev)->detail);

					break;
				case XCB_KEY_RELEASE:
					key_release(((xcb_key_press_event_t*)ev)->detail);

					break;
			}

			free(ev);
		}

		set_uniforms();
		update_screen();
		update();

		long now_time = millis();
		delta = (float)(now_time - last_time) / 1000.;
		float fps = 1. / delta;
		//printf("%f\n", fps);
		last_time = now_time;
	}

	free_window();
}


void get_mouse_pos(int* x, int* y) {
	xcb_query_pointer_cookie_t pointer_cookie = xcb_query_pointer(dpy, screen->root);
	xcb_query_pointer_reply_t *pointer_reply = xcb_query_pointer_reply(dpy, pointer_cookie, NULL);

	*x = pointer_reply->root_x;
	*y = pointer_reply->root_y;

	free(pointer_reply);
}


void key_press(int keycode) {
	if (keycode == 24) // Q
		camera_vel.y = move_speed;

	if (keycode == 26) // E
		camera_vel.y = -move_speed;

	if (keycode == 38) // A
		camera_vel.x = -move_speed;

	if (keycode == 25) // W
		camera_vel.z = move_speed;

	if (keycode == 39) // S
		camera_vel.z = -move_speed;

	if (keycode == 40) // D
		camera_vel.x = move_speed;
}

void key_release(int keycode) {
	if (keycode == 24) // Q
		camera_vel.y = 0;

	if (keycode == 26) // E
		camera_vel.y = 0;

	if (keycode == 38) // A
		camera_vel.x = 0;

	if (keycode == 25) // W
		camera_vel.z = 0;

	if (keycode == 39) // S
		camera_vel.z = 0;

	if (keycode == 40) // D
		camera_vel.x = 0;
}


long millis() {
	struct timeval tv;

	gettimeofday(&tv,NULL);
	return (((long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}


void update() {
	get_mouse_pos(&mouse_vel_x,&mouse_vel_y);
	mouse_vel_x -= 100;
	mouse_vel_y -= 100;
	camera_rot.y -= (float)mouse_vel_x * rot_speed;
	camera_rot.x += (float)mouse_vel_y * rot_speed;
	update_camera_rot_mat();
	camera_pos = vsum(camera_pos, vmulm(vmulf(camera_vel, delta), camera_rot_mat));
}
