#include <render.h>
#include <shaders.h>
#include <window.h>
#include <model.h>
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
int data_buffer;
int data_tex;


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
	glBindTexture(GL_TEXTURE_2D, outTex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, window_width, window_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, outTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glTexStorage2D(GL_TEXTURE_2D, 8, GL_RGBA32F, window_width,window_height);


	glGenTextures(1, &data_tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, data_tex);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, size,size,size, 0, GL_RGBA, GL_FLOAT, data);
	glGenerateMipmap(GL_TEXTURE_3D);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);//GL_NEAREST_MIPMAP_NEAREST);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
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




void render_model(Model model) {
	glUseProgram(comp_prog);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, data_tex);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, model.resolution,model.resolution,model.resolution, 0, GL_RGBA, GL_FLOAT, model.data);
	glGenerateMipmap(GL_TEXTURE_3D);
	glDispatchCompute((GLuint)window_width, (GLuint)window_height, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


	//free(array);
}
