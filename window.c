#include <window.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>
#include <GL/glx.h>
#include <GL/gl.h>


int window_width;
int window_height;

Display* display;
xcb_connection_t* dpy;
xcb_window_t window;
xcb_screen_t* screen;
int screen_id;

xcb_atom_t delte_window_atom;
xcb_atom_t protocols_atom;

GLXWindow glxwindow;
GLXFBConfig fb_config;
GLXContext context;

struct WMSizeHints {
	uint32_t flags;
	int32_t  x, y;
	int32_t  width, height;
	int32_t  min_width, min_height;
	int32_t  max_width, max_height;
	int32_t  width_inc, height_inc;
	int32_t  min_aspect_num, min_aspect_den;
	int32_t  max_aspect_num, max_aspect_den;
	int32_t  base_width, base_height;
	uint32_t win_gravity;
};


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


void init_atoms();
void create_window();
void configure_window();
void create_glx_window();


void init_window(int width, int height) {
	window_width  = width;
	window_height = height;

	display = XOpenDisplay(0);
	screen_id = DefaultScreen(display);
	dpy = XGetXCBConnection(display);

	if (dpy == NULL) {
		perror("XGetXCBConnection");
		exit(errno);
	}

	screen = xcb_setup_roots_iterator(xcb_get_setup(dpy)).data;

	init_atoms();
	create_window();
	configure_window();
	create_glx_window();
	xcb_flush(dpy);
}


void init_atoms() {
	xcb_intern_atom_cookie_t protocols_atom_cookie = xcb_intern_atom(dpy, 0, 12, "WM_PROTOCOLS");
	xcb_intern_atom_reply_t* protocols_atom_reply = xcb_intern_atom_reply(dpy, protocols_atom_cookie, NULL);

	xcb_intern_atom_cookie_t delte_window_atom_cookie = xcb_intern_atom(dpy, 0, 16, "WM_DELETE_WINDOW");
	xcb_intern_atom_reply_t* delte_window_atom_reply = xcb_intern_atom_reply(dpy, delte_window_atom_cookie, NULL);

	delte_window_atom = delte_window_atom_reply->atom;
	protocols_atom = protocols_atom_reply->atom;

	free(protocols_atom_reply);
	free(delte_window_atom_reply);
}


void create_window() {
	window = xcb_generate_id(dpy);

	int mask = XCB_CW_EVENT_MASK;
	uint32_t values[] = {
		XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_POINTER_MOTION
	};

	xcb_create_window(
			dpy, XCB_COPY_FROM_PARENT, window, screen->root,
			0, 0, window_width,window_height, 0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, values);

	xcb_map_window(dpy, window);
}


void configure_window() {
	struct WMSizeHints normal_hints;

	normal_hints.min_width  = window_width;
	normal_hints.min_height = window_height;
	normal_hints.max_width  = window_width;
	normal_hints.min_height = window_height;

	xcb_change_property(dpy, XCB_PROP_MODE_REPLACE, window, protocols_atom, XCB_ATOM_ATOM, 32, 1, &delte_window_atom);
	xcb_change_property(dpy, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NORMAL_HINTS, XCB_ATOM_WM_SIZE_HINTS, 32, sizeof(struct WMSizeHints) >> 2, &normal_hints);

	xcb_grab_pointer_cookie_t grab_cookie = xcb_grab_pointer(dpy, 0, window, XCB_EVENT_MASK_POINTER_MOTION, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, window, XCB_NONE, XCB_CURRENT_TIME);
	xcb_grab_pointer_reply_t *grab_reply = xcb_grab_pointer_reply(dpy, grab_cookie, NULL);
	free(grab_reply);

	//xcb_xfixes_query_version(dpy, 4,0);
	//xcb_xfixes_hide_cursor(dpy, screen->root);
}


void create_glx_window() {
	GLXFBConfig* fb_configs;
	int num_fb_configs;

	fb_configs = glXChooseFBConfig(display, screen_id, visual_attribs, &num_fb_configs);

	fb_config = fb_configs[0];

	int visualID;
	glXGetFBConfigAttrib(display, fb_config, GLX_VISUAL_ID , &visualID);

	glxwindow = glXCreateWindow(display, fb_config, window, 0);

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
}


void free_window() {
	glXDestroyWindow(display, glxwindow);
	glXDestroyContext(display, context);
	xcb_destroy_window(dpy, window);
	xcb_disconnect(dpy);
	XCloseDisplay(display);
}
