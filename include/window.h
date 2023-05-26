#ifndef WINDOW_H
#define WINDOW_H


#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>
#include <GL/glx.h>


extern int window_width;
extern int window_height;

extern Display* display;
extern xcb_connection_t* dpy;
//extern xcb_window_t window;
extern xcb_screen_t* screen;
//extern int screen_id;

extern GLXWindow glxwindow;

extern xcb_atom_t delte_window_atom;


void init_window(int,int);
void free_window();


#endif // WINDOW_H
