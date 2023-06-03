#include <obj_parser.h>
#include <utils.h>
#include <png.h>
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






struct ONode;

typedef struct ONode {
	struct ONode* childs[8];
	void* data;
	unsigned char data_dist; // 1 - data in this node
} ONode;

typedef struct {
	ONode* root;
	char levels;
} SVO;


SVO init_svo(char levels) {
	return (SVO){calloc(sizeof(ONode),1), levels};
}


void free_node(ONode* node) {
	if (node == NULL)
		return;

	for (int i = 0; i < 8; i++)
		free_node(node->childs[i]);

	if (node->data != NULL)
		free(node->data);

	free(node);
}


void free_svo(SVO tree) {
	free_node(tree.root);
	tree.levels = 0;
}


char add_data(SVO* tree, unsigned char level, char ids[], void* data) {
	if (level > 16)
		level = 16;

	ONode* root = tree->root;
	if (level > tree->levels)
		return 0;

	for (int i = level-1; i >= 0; i--) {
		if (root->childs[ids[i]] == NULL)
			root->childs[ids[i]] = calloc(sizeof(ONode),1);
		else if (root->childs[ids[i]]->data != NULL) {
			printf("error: try to add node to leaf\n");
			return 0;
		}

		if (root->data_dist == 0 || level-i+1 < root->data_dist)
			root->data_dist = level-i+1;
		root = root->childs[ids[i]];
	}

	if (root->data != NULL)
		return 0;

	root->data = data;
	root->data_dist = 1;

	return 1;
}


void coords_to_ids(short x, short y, short z, char ids[]) {
	//printf("%d %d %d\n", x,y,z);
	for (int i = 0; i < 16; i++) {
		ids[i] = (((z >> i) & 1) << 2) | (((y >> i) & 1) << 1) | ((x >> i) & 1);
		//printf("%d ", ids[i]);
	}
	//putc('\n', stdout);
	//putc('\n', stdout);
}



void print_node(ONode* node, int level) {
	for (int i = 0; i < level; i++)
		putc('\t', stdout);

	printf("%d %p", level, node);

	if (node == NULL) {
		putc('\n', stdout);
		return;
	}

	printf(" %u %p\n", node->data_dist, node->data);

	for (int i = 0; i < 8; i++) {
		print_node(node->childs[i], level+1);
	}
}


void print_svo(SVO tree) {
	printf("%d\n", tree.levels);
	print_node(tree.root, 0);
}


void save_svo_to_array(SVO tree, unsigned int** array, int* size) {
	ONode** queue = malloc(sizeof(ONode*));
	int queue_size = 1;

	queue[0] = tree.root;

	while (queue_size > 0) {
		ONode* node = queue[0];
		queue_size--;
https://habr.com/ru/articles/350782/
		ONode** queue_new = malloc(sizeof(ONode*)*queue_size);
		memcpy(queue_new, &queue[1], sizeof(ONode*)*queue_size);
		free(queue);
		queue = queue_new;

		char childs = 0;
		char leafs = 0;

		for (int i = 0; i < 8; i++) {
			if (node->childs[i] == NULL)
				continue;

			if (node->childs[i]->data_dist == 1)
				leafs |= 1 << i;

			childs |= 1 << i;
		}

		if (childs == 0)
			continue;

		(*size)++;
		*array = realloc(*array, (*size)*sizeof(unsigned int));

		unsigned short fst = *size + queue_size;

		(*array)[(*size)-1] = ((fst & 0xffff) << 16) | ((childs & 0xff) << 8) | (leafs & 0xff);

		//printf("%u %u %u %u %u\n", *size, queue_size, fst, childs&255, leafs&255);

		for (int i = 0; i < 8; i++) {
			if (node->childs[i] == NULL)
				continue;

			queue_size++;
			queue = realloc(queue, sizeof(ONode*)*queue_size);
			queue[queue_size-1] = node->childs[i];
		}
	}

	free(queue);
}






const int levels = 8;

const int resolution = 1<<levels;
const float fres = resolution;


char* input_filename = NULL;
char* output_filename = "object.mdl";


ObjParsedFile parsed_file;

int* faces;
int faces_count;


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
	//GLX_SAMPLE_BUFFERS  , 1,
	//GLX_SAMPLES         , 4,
};


void print_help(char*);
void init_window();
void init_shaders_vox();
void save_output();



SVO tree;




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

	tree = init_svo(levels);

	image = calloc(resolution*resolution*4*4,1);
	voxel_pos = calloc(resolution*resolution*resolution*4*4,1);

	parsed_file = parse_file(input_filename);

	// triangulate
	faces = malloc(0);

	for (int i = 0; i < parsed_file.faces_count; i++) {
		obj_face* face = &parsed_file.faces[i];

		for (int j = 0; j < face->count - 2; j++) {
			faces = realloc(faces, sizeof(int) * 3 * (++faces_count));

			faces[(faces_count-1)*3+0] = face->vertexes[j+0];
			faces[(faces_count-1)*3+1] = face->vertexes[j+1];
			faces[(faces_count-1)*3+2] = face->vertexes[j+2];
		}
	}


	float min_x = INFINITY;
	float max_x =-INFINITY;
	float min_y = INFINITY;
	float max_y =-INFINITY;
	float min_z = INFINITY;
	float max_z =-INFINITY;

	for (int i = 0; i < parsed_file.vertexes_count; i++) {
		min_x = fmin(min_x, parsed_file.vertexes[i].x);
		max_x = fmax(max_x, parsed_file.vertexes[i].x);
		min_y = fmin(min_y, parsed_file.vertexes[i].y);
		max_y = fmax(max_y, parsed_file.vertexes[i].y);
		min_z = fmin(min_z, parsed_file.vertexes[i].z);
		max_z = fmax(max_z, parsed_file.vertexes[i].z);
	}


	init_window();
	init_shaders_vox();


	float scalex = 2./(max_x-min_x);
	float scaley = 2./(max_y-min_y);
	float scalez = 2./(max_z-min_z);

	float scale = fmin(fmin(scalex, scaley), scalez);

	float offset[3] = {
		(max_x+min_x)*0.5,
		(max_y+min_y)*0.5,
		(max_z+min_z)*0.5,
	};

	printf("%f %f %f %f\n", scale, offset[0], offset[1], offset[2]);



	glUseProgram(vox_prog);
	glUniform1fv(5, 1, &fres);
	glUniform1fv(6, 1, &scale);
	glUniform3fv(7, 1, &offset);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, outTex);

	glDrawElements(GL_TRIANGLES, parsed_file.faces_count * 3, GL_UNSIGNED_INT, 0);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glReadPixels(0,0,resolution,resolution, GL_RGBA, GL_FLOAT, image);


	glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, voxel_pos);

	int vc = 0;

	for (int i = 0; i < resolution*resolution*resolution; i++) {
		//printf("%f %f %f %f\n", voxel_pos[i*4+0], voxel_pos[i*4+1], voxel_pos[i*4+2], voxel_pos[i*4+3]);
		if (voxel_pos[i*4+3] == 0)
			continue;

		vc++;

		int x = i % resolution;
		int y = (i / resolution) % resolution;
		int z = i / resolution / resolution;

		char ids[16];
		coords_to_ids(x,y,z, ids);
		add_data(&tree, levels, ids, malloc(0));
	}

	//printf("%d\n", vc);



	//print_svo(tree);


	save_output("output.png");


	int size;
	unsigned int* array = malloc(0);

	save_svo_to_array(tree, &array, &size);

	printf("----------------------------\n");

	size *= 4;

	/*for (int i = 0; i < size/4; i++) {
		printf("%d %d %d\n", array[i]>>16, (array[i]&0xff00)>>8, array[i] & 0xff);
	}*/


	int f = open(output_filename, O_WRONLY|O_CREAT|O_TRUNC, 0644);

	if (!f) {
		perror("open");
		exit(errno);
	}

	write(f, &size, sizeof(size));

	write(f, array, size);

	close(f);


	free(faces);
	free_svo(tree);
	free(array);
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
	glBufferData(GL_ARRAY_BUFFER, parsed_file.vertexes_count * sizeof(vec3f), parsed_file.vertexes, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);


	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces_count * sizeof(int) * 3, faces, GL_STATIC_DRAW);



	glGenTextures(1, &outTex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, outTex);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, resolution, resolution, resolution, 0, GL_RGBA, GL_FLOAT, voxel_pos);
	glBindImageTexture(0, outTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

	glTexStorage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, resolution,resolution,resolution);
}


void save_output(char* name) {
	FILE* f = fopen(name, "wb");

	if (!f) {
		perror("fopen");
		exit(errno);
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr) {
		perror("png_create_write_struct");
		exit(errno);
	}


	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr) {
		perror("png_create_info_struct");
		exit(errno);
	}


	unsigned char** image2 = malloc(resolution * sizeof(char*));

	for (int i = 0; i < resolution; i++) {
		image2[i] = malloc(resolution*3);

		//memcpy(image2[i], &image[i*resolution*3], width*3);

		for (int j = 0; j < resolution*3; j+=3) {
			image2[i][j+0] = image[i*resolution*4+j/3*4+0]*255;
			image2[i][j+1] = image[i*resolution*4+j/3*4+1]*255;
			image2[i][j+2] = image[i*resolution*4+j/3*4+2]*255;
		}
	}


	png_init_io(png_ptr, f);

	png_set_IHDR(png_ptr, info_ptr, resolution, resolution, 8,
	  PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
	  PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);


	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, (unsigned char**)image2);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);


	fclose(f);


	for (int i = 0; i < resolution; i++)
		free(image2[i]);

	free(image2);
}
