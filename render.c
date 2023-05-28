#include <render.h>
#include <shaders.h>
#include <window.h>
#include <model.h>
#include <svo.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>



vec3f camera_pos = {0,0,0};
vec3f camera_rot = {0,0,0};
mat3 camera_rot_mat;



float vertices[] = {
	 1, 1,
	-1, 1,
	 1,-1,
	-1,-1,
};

int VBO;
int VAO;

int outTex;


void init_render() {
	glViewport(0,0, window_width,window_height);


	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);


	glGenTextures(1, &outTex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, outTex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, window_width, window_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, outTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glTexStorage2D(GL_TEXTURE_2D, 8, GL_RGBA32F, window_width,window_height);
}


void set_uniforms() {
	glUseProgram(comp_prog);

	float res[] = {window_width,window_height};

	glUniform2fv(0, 1, (const GLfloat*)res);
	glUniform3fv(1, 1, (const GLfloat*)&camera_pos);
	glUniformMatrix3fv(2, 1, GL_FALSE, (const GLfloat*)&camera_rot_mat);


	glUseProgram(render_prog);

	glUniform2fv(0, 1, (const GLfloat*)res);
}


void update_screen() {
	glUseProgram(render_prog);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, outTex);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glXSwapBuffers(display, glxwindow);
}


void update_camera_rot_mat() {
	float cx = cos(camera_rot.x);
	float sx = sin(camera_rot.x);
	float cy = cos(camera_rot.y);
	float sy = sin(camera_rot.y);

	mat3 rot_x = {
		 1,  0,  0,
		 0, cx,-sx,
		 0, sx, cx
	};

	mat3 rot_y = {
		 cy, 0,-sy,
		  0, 1,  0,
		 sy, 0, cy
	};

	camera_rot_mat = vmmul(rot_y,rot_x);
}



void save_node_to_array(ONode*, unsigned char**, int*);

void save_node_to_array(ONode* node, unsigned char** array, int* size) {
	unsigned char childs = 0;
	char childs_count = 0;

	for (int i = 0; i < 8; i++) {
		if (node->childs[i] != NULL) {
			childs |= 1 << i;
			childs_count++;
		}
	}

	if (childs == 0) {
		if (node->data == NULL)
			return;
		Voxel* v = node->data;
		*size += 3 + 1;
		*array = realloc(*array, *size);
		(*array)[*size - 3 - 1] = 0;
		(*array)[*size - 3 + 0] = v->color[0];
		(*array)[*size - 3 + 1] = v->color[1];
		(*array)[*size - 3 + 2] = v->color[2];
		return;
	}

	*size += childs_count + 1;
	*array = realloc(*array, *size);

	(*array)[*size - childs_count - 1] = childs;

	int fst = *size - childs_count;

	for (int i = 0; i < 8; i++) {
		if (node->childs[i] == NULL)
			continue;

		int last_size = *size;

		save_node_to_array(node->childs[i], array, size);

		(*array)[fst++] = last_size;

		last_size = *size;
	}
}



unsigned char* array = NULL;
int size;



void render_model(Model model) {
	if (array == NULL) {
		array = malloc(0);

		save_node_to_array(model.tree.root, &array, &size);

		/*for (int i = 0; i < size; i++) {
			if ((i&3) == 0)
				putc('\n', stdout);
			printf("%d ", array[i]);
		}

		fflush(stdout);*/

		//exit(0);
	}

	/*unsigned char array[] = {
		3, 3,7,
		0, 255,0,0,
		0, 0,255,0
	};
	int size = sizeof(array);*/


	glUseProgram(comp_prog);

	//glUniform1iv(3, 1, (const GLint*)&tree_size);
	glUniform1iv(4, (size+3)/4, (const GLuint*)array);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, outTex);
	glDispatchCompute((GLuint)window_width, (GLuint)window_height, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


	//free(array);
}
