#include <utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>

#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>
#include <GL/glx.h>
#include <GL/gl.h>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <tinyobj_loader_c.h>



const int levels = 6;

const int resolution = 1<<levels;
const float fres = resolution;


char* input_filename = NULL;
char* output_filename = "object.mdl";

int* faces;
char* data_free;


Display* display;
xcb_connection_t* dpy;
xcb_window_t window;
xcb_screen_t* screen;
int screen_id;

GLXWindow glxwindow;
GLXFBConfig fb_config;
GLXContext context;


char* vert_shader_source;
char* frag_shader_source;
char* geom_shader_source;

int vert_shader;
int frag_shader;
int geom_shader;

int vox_prog;

int outTex;

int VBO;
int VAO;
int EBO;

float* image;
float* voxel_pos;


static int visual_attribs[] = {
	GLX_X_RENDERABLE, True,
	GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
	GLX_RENDER_TYPE, GLX_RGBA_BIT,
	GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
	GLX_RED_SIZE, 8,
	GLX_GREEN_SIZE, 8,
	GLX_BLUE_SIZE, 8,
	GLX_ALPHA_SIZE, 8,
	GLX_DEPTH_SIZE, 24,
	GLX_STENCIL_SIZE, 8,
	GLX_DOUBLEBUFFER, True,
	//GLX_SAMPLE_BUFFERS	, 1,
	//GLX_SAMPLES				 , 4,
};


tinyobj_attrib_t attrib;

tinyobj_shape_t* shapes;
long shapes_count;

tinyobj_material_t* materials;
long materials_count;


void print_help(char*);
void init_window();
void init_shaders_vox();

void get_file_data(void*, const char*, const int,const char*, char**, size_t*);


int main(int argc, char** argv) {
	if (argc == 1)
		print_help(argv[0]);

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0)
			input_filename = argv[++i];
		else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0)
			output_filename = argv[++i];
		else {
			if (input_filename == NULL)
				input_filename = argv[i];
			else
				output_filename = argv[i];
		}
	}

	voxel_pos = calloc(resolution*resolution*resolution*4*4,1);


	tinyobj_attrib_init(&attrib);


	tinyobj_parse_obj(&attrib,
			&shapes, &shapes_count,
			&materials, &materials_count,
			input_filename, get_file_data,
			NULL, TINYOBJ_FLAG_TRIANGULATE);


	//printf("%d %d %d\n", attrib.num_vertices, attrib.num_faces, attrib.num_face_num_verts);

	faces = malloc(attrib.num_faces * 4);

	for (int i = 0; i < attrib.num_faces; i++)
		faces[i] = attrib.faces[i].v_idx;


	float min_x = INFINITY;
	float max_x =-INFINITY;
	float min_y = INFINITY;
	float max_y =-INFINITY;
	float min_z = INFINITY;
	float max_z =-INFINITY;


	for (int i = 0; i < attrib.num_vertices; i++) {
		min_x = fmin(min_x, attrib.vertices[i*3+0]);
		max_x = fmax(max_x, attrib.vertices[i*3+0]);
		min_y = fmin(min_y, attrib.vertices[i*3+1]);
		max_y = fmax(max_y, attrib.vertices[i*3+1]);
		min_z = fmin(min_z, attrib.vertices[i*3+2]);
		max_z = fmax(max_z, attrib.vertices[i*3+2]);
	}


	float scalex = 2./(max_x-min_x);
	float scaley = 2./(max_y-min_y);
	float scalez = 2./(max_z-min_z);

	float scale = fmin(fmin(scalex, scaley), scalez);

	float offset[3] = {
		(max_x+min_x)*0.5,
		(max_y+min_y)*0.5,
		(max_z+min_z)*0.5,
	};


	init_window();
	init_shaders_vox();

	glUseProgram(vox_prog);
	glUniform1fv(5, 1, &fres);
	glUniform1fv(6, 1, &scale);
	glUniform3fv(7, 1, &offset);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, outTex);

	glDrawElements(GL_TRIANGLES, attrib.num_faces, GL_UNSIGNED_INT, 0);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, voxel_pos);


	int f = open(output_filename, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if (!f) {
		perror("open");
		exit(errno);
	}
	int size = resolution*resolution*resolution*4*4;
	write(f, &resolution, sizeof(resolution));
	write(f, voxel_pos, size);
	close(f);


	free(faces);
	tinyobj_attrib_free(&attrib);
	tinyobj_shapes_free(shapes, shapes_count);
	tinyobj_materials_free(materials, materials_count);
	free(data_free);
}


void print_help(char* name) {
	printf("Usage: %s <filename.obj> <filename.mdl>\n", name);
	exit(0);
}


void init_window() {
	display = XOpenDisplay(0);
	screen_id = DefaultScreen(display);
	dpy = XGetXCBConnection(display);

	if (dpy == NULL) {
		perror("XGetXCBConnection");
		exit(errno);
	}

	screen = xcb_setup_roots_iterator(xcb_get_setup(dpy)).data;


	GLXFBConfig* fb_configs;
	int num_fb_configs;

	fb_configs = glXChooseFBConfig(display, screen_id, visual_attribs, &num_fb_configs);

	fb_config = fb_configs[0];

	int visualID;
	glXGetFBConfigAttrib(display, fb_config, GLX_VISUAL_ID , &visualID);

	glxwindow = glXCreateWindow(display, fb_config, screen->root, 0);

	context = glXCreateNewContext(display, fb_config, GLX_RGBA_TYPE, 0, 1);

	if (!context) {
		perror("glXCreateNewContext");
		exit(errno);
	}

	if(!glXMakeContextCurrent(display, glxwindow, glxwindow, context)) {
		perror("glXMakeContextCurrent");
		exit(errno);
	}

	free(fb_configs);


	glEnable(GL_TEXTURE_3D);
	glDisable(GL_DEPTH_TEST);
}


void init_shaders_vox() {
	int tmp;

	read_file("voxelizer.vert", &vert_shader_source, &tmp);
	read_file("voxelizer.frag", &frag_shader_source, &tmp);
	read_file("voxelizer.geom", &geom_shader_source, &tmp);

	vert_shader = create_shader(vert_shader_source, GL_VERTEX_SHADER);
	frag_shader = create_shader(frag_shader_source, GL_FRAGMENT_SHADER);
	geom_shader = create_shader(geom_shader_source, GL_GEOMETRY_SHADER);

	vox_prog = create_program(3, (int[]){vert_shader, frag_shader, geom_shader});


	glViewport(0,0, resolution,resolution);


	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, attrib.num_vertices * 4 * 3, attrib.vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);


	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, attrib.num_faces*4, faces, GL_STATIC_DRAW);



	glGenTextures(1, &outTex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, outTex);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, resolution, resolution, resolution, 0, GL_RGBA, GL_FLOAT, voxel_pos);
	glBindImageTexture(0, outTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

	glTexStorage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, resolution,resolution,resolution);
}


void get_file_data(void* ctx, const char* filename, const int is_mtl,const char* obj_filename, char** data, size_t* len) {
	(void)ctx;

	int f = open(obj_filename, O_RDONLY);

	if (f == -1) {
		perror("open");
		exit(errno);
	}

	struct stat st;
	fstat(f, &st);
	*len = st.st_size+1;

	*data = calloc(*len,1);

	read(f, *data, *len);

	data_free = *data;

	close(f);
}
