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



void save_node_to_array(ONode*, unsigned int**, int*);

void save_node_to_array(ONode* node, unsigned int** array, int* size) {
	char childs = 0;

	for (int i = 0; i < 8; i++)
		if (node->childs[i] != NULL)
			childs |= 1 << i;

	if (childs == 0)
		return;

	char leafs = 0;

	for (int i = 0; i < 8; i++) {
		if (node->childs[i] == NULL)
			continue;

		if (node->childs[i]->data_dist == 1)
			leafs |= 1 << i;
	}

	(*size)++;
	*array = realloc(*array, (*size)*4);

	unsigned int value = (*size << 16) | (childs << 8) | leafs;

	(*array)[*size-1] = value;

	for (int i = 0; i < 8; i++)
		if (node->childs[i] != NULL)
			save_node_to_array(node->childs[i], array, size);
}



//unsigned int* array = NULL;
//int size;


void render_model(Model model) {
	/*if (array == NULL) {
		size = 0;
		array = malloc(0);

		save_node_to_array(model.tree.root, &array, &size);


		/*printf("%d\n", size);

		for (int i = 0; i < size; i++)
			printf("%u %u %u\n", array[i] >> 16, (array[i] & 0xff00) >> 8, array[i] & 0xff);*
	}*/

	unsigned int array[] = {66046u,196352u,719365u,1111306u,1504267u,1798287u,2010960u,2383776u,2760672u,3081212u,3244159u,3317855u,3448927u,3604224u,4127237u,4521728u,4997311u,5075087u,5308160u,5821483u,6094592u,6583690u,6924885u,7205389u,7533324u,7879365u,8190469u,8581900u,8917486u,9045243u,9153109u,9436928u,9961216u,10485504u,10945277u,11012853u,11206400u,11685298u,11992832u,12474280u,12782835u,12914672u,13238016u,13729664u,14169036u,14418430u,14484221u,14565312u,14946798u,33023u,57599u,12543u,52479u,41727u,51455u,52479u,8447u,65535u,52479u,52479u,65535u,65535u,57599u,61695u,35071u,65535u,43775u,65535u,61695u,65535u,62207u,65535u,65535u,65535u,65535u,65535u,49407u,61695u,20735u,12543u,52479u,50431u,65535u,65535u,52479u,52479u,65535u,65535u,20735u,50431u,65535u,54783u,65535u,65535u,65535u,65535u,65535u,65535u,65535u,65535u,62975u,28927u,65535u,12543u,65535u,33023u,61695u,56831u,14335u,12543u,60159u,65535u,47871u,65535u,61695u,61695u,65535u,65535u,29695u,12543u,65535u,511u,8447u,47103u,49407u,63743u,65535u,19711u,8959u,1535u,61695u,61695u,65535u,65535u,13311u,12799u,4351u,30719u,5631u,35071u,35071u,35071u,767u,52479u,52479u,65535u,65535u,52479u,52479u,8191u,49151u,35583u,65535u,44031u,65535u,2303u,36863u,2815u,45055u,65535u,65535u,65535u,65535u,53247u,61439u,65535u,65535u,3327u,2815u,767u,52479u,52479u,65535u,65535u,52479u,52479u,65535u,65535u,52479u,65535u,56831u,19967u,65535u,65535u,65535u,65535u,65535u,65535u,65535u,65535u,65535u,13311u,65535u,24575u,24575u,3327u,1279u,4095u,3583u,4095u,511u,61183u,65535u,65535u,65535u,61183u,57343u,49151u,32767u,65535u,65535u,22527u,1023u,4095u,4095u,511u,13311u,13311u,767u,1023u,4607u,1279u,65535u,65535u,13311u,13311u,16383u,8191u,30719u,511u};
	int size = sizeof(array);


	glUseProgram(comp_prog);

	glUniform1iv(3, size, (const GLint*)array);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, outTex);
	glDispatchCompute((GLuint)window_width, (GLuint)window_height, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


	//free(array);
}
