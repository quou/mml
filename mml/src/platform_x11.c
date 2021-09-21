#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/glx.h>

#include "mml.h"
#include "keytable.h"

struct mml_window {
	int w, h;

	int flags;

	Display* display;
	Window window;
	XVisualInfo* visual;
	XSetWindowAttributes attribs;
	GLXContext context;

	int pixel_size;
	unsigned int gl_tex;
	struct mml_bitmap* backbuffer;

	struct mml_key_table keymap;
};

struct mml_window* mml_new_window(const char* name, int w, int h, int pixel_size, int flags) {
	struct mml_window* window;
	int screen_id;
	Atom delete_window, wm_state, fullscreen;
	XEvent xev;
	XWindowAttributes gwa;
	XSizeHints* size_hints;
	void (*swap_interval_ext)(Display*, Window, int);
	int glx_attribs[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE,     24,
		GLX_STENCIL_SIZE,   8,
		GLX_RED_SIZE,       8,
		GLX_GREEN_SIZE,     8,
		GLX_BLUE_SIZE,      8,
		GLX_SAMPLE_BUFFERS, 0,
		GLX_SAMPLES,        0,
		None
	};

	window = malloc(sizeof(struct mml_window));
	window->flags = flags;
	window->w = w;
	window->h = h;

	window->display = XOpenDisplay(NULL);
	screen_id = DefaultScreen(window->display);

	window->visual = glXChooseVisual(window->display, screen_id, glx_attribs);

	window->attribs.border_pixel = BlackPixel(window->display, screen_id);
	window->attribs.background_pixel = BlackPixel(window->display, screen_id);
	window->attribs.override_redirect = True;
	window->attribs.event_mask = 	ExposureMask 		|
					KeyPressMask 		|
					KeyReleaseMask 		|
					ButtonPressMask		|
					ButtonReleaseMask	|
					FocusChangeMask 	|
					PointerMotionMask;
	window->attribs.colormap = XCreateColormap(window->display, RootWindow(window->display, screen_id),
			window->visual->visual, AllocNone);
	window->window = XCreateWindow(window->display, RootWindow(window->display, screen_id), 0, 0,
			w * pixel_size, h * pixel_size, 0,
			window->visual->depth, InputOutput, window->visual->visual,
			CWBackPixel | CWColormap | CWBorderPixel | CWEventMask,
			&window->attribs);

	delete_window = XInternAtom(window->display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(window->display, window->window, &delete_window, 1);

	if (!(flags & MML_WINDOW_RESIZABLE)) {
		/* This works by setting the miniumum and maximum heights of the window
		 * to the input width and height. I'm not sure if this is the correct
		 * way to do it, but it works, and even removes the maximise button */
		size_hints = XAllocSizeHints();
		size_hints->flags = PMinSize | PMaxSize;
		size_hints->min_width = w * pixel_size;
		size_hints->min_height = h * pixel_size;
		size_hints->max_width = w * pixel_size;
		size_hints->max_height = h * pixel_size;

		XSetWMNormalHints(window->display, window->window, size_hints);

		XFree(size_hints);
	}

	if (flags & MML_WINDOW_FULLSCREEN) {
		wm_state = XInternAtom(window->display, "_NET_WM_STATE", False);
		fullscreen = XInternAtom(window->display, "_NET_WM_STATE_FULLSCREEN", False);
		xev.type = ClientMessage;
		xev.xclient.window = window->window;
		xev.xclient.message_type = wm_state;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 1;
		xev.xclient.data.l[1] = fullscreen;
		xev.xclient.data.l[2] = 0;
		xev.xclient.data.l[3] = 0;
		XMapWindow(window->display, window->window);
		XSendEvent(window->display, DefaultRootWindow(window->display), False,
			SubstructureRedirectMask | SubstructureNotifyMask, &xev);
		XFlush(window->display);
	}

	XGetWindowAttributes(window->display, window->window, &gwa);
	window->w = gwa.width / pixel_size;
	window->h = gwa.height / pixel_size;

	window->context = glXCreateContext(window->display, window->visual, NULL, GL_TRUE);
	glXMakeCurrent(window->display, window->window, window->context);

	XClearWindow(window->display, window->window);
	XMapRaised(window->display, window->window);

	swap_interval_ext = glXGetProcAddress((unsigned char*)"glXSwapIntervalEXT");

	if (swap_interval_ext) {
		swap_interval_ext(window->display, window->window, 0);
	}

	if (name) {
		XStoreName(window->display, window->window, name);
	}

	window->pixel_size = pixel_size;

	window->backbuffer = mml_new_bitmap(w, h);

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &window->gl_tex);
	glBindTexture(GL_TEXTURE_2D, window->gl_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, window->backbuffer->pixels);

	init_key_table(&window->keymap);

	/* Key mapping */
	key_table_insert(&window->keymap, 0x00, MML_KEY_UNKNOWN);
	key_table_insert(&window->keymap, 0x61, MML_KEY_A);
	key_table_insert(&window->keymap, 0x62, MML_KEY_B);
	key_table_insert(&window->keymap, 0x63, MML_KEY_C);
	key_table_insert(&window->keymap, 0x64, MML_KEY_D);
	key_table_insert(&window->keymap, 0x65, MML_KEY_E);
	key_table_insert(&window->keymap, 0x66, MML_KEY_F);
	key_table_insert(&window->keymap, 0x67, MML_KEY_G);
	key_table_insert(&window->keymap, 0x68, MML_KEY_H);
	key_table_insert(&window->keymap, 0x69, MML_KEY_I);
	key_table_insert(&window->keymap, 0x6A, MML_KEY_J);
	key_table_insert(&window->keymap, 0x6B, MML_KEY_K);
	key_table_insert(&window->keymap, 0x6C, MML_KEY_L);
	key_table_insert(&window->keymap, 0x6D, MML_KEY_M);
	key_table_insert(&window->keymap, 0x6E, MML_KEY_N);
	key_table_insert(&window->keymap, 0x6F, MML_KEY_O);
	key_table_insert(&window->keymap, 0x70, MML_KEY_P);
	key_table_insert(&window->keymap, 0x71, MML_KEY_Q);
	key_table_insert(&window->keymap, 0x72, MML_KEY_R);
	key_table_insert(&window->keymap, 0x73, MML_KEY_S);
	key_table_insert(&window->keymap, 0x74, MML_KEY_T);
	key_table_insert(&window->keymap, 0x75, MML_KEY_U);
	key_table_insert(&window->keymap, 0x76, MML_KEY_V);
	key_table_insert(&window->keymap, 0x77, MML_KEY_W);
	key_table_insert(&window->keymap, 0x78, MML_KEY_X);
	key_table_insert(&window->keymap, 0x79, MML_KEY_Y);
	key_table_insert(&window->keymap, 0x7A, MML_KEY_Z);
	key_table_insert(&window->keymap, XK_F1, MML_KEY_F1);
	key_table_insert(&window->keymap, XK_F2, MML_KEY_F2);
	key_table_insert(&window->keymap, XK_F3, MML_KEY_F3);
	key_table_insert(&window->keymap, XK_F4, MML_KEY_F4);
	key_table_insert(&window->keymap, XK_F5, MML_KEY_F5);
	key_table_insert(&window->keymap, XK_F6, MML_KEY_F6);
	key_table_insert(&window->keymap, XK_F7, MML_KEY_F7);
	key_table_insert(&window->keymap, XK_F8, MML_KEY_F8);
	key_table_insert(&window->keymap, XK_F9, MML_KEY_F9);
	key_table_insert(&window->keymap, XK_F10, MML_KEY_F10);
	key_table_insert(&window->keymap, XK_F11, MML_KEY_F11);
	key_table_insert(&window->keymap, XK_F12, MML_KEY_F12);
	key_table_insert(&window->keymap, XK_Down, MML_KEY_DOWN);
	key_table_insert(&window->keymap, XK_Left, MML_KEY_LEFT);
	key_table_insert(&window->keymap, XK_Right, MML_KEY_RIGHT);
	key_table_insert(&window->keymap, XK_Up, MML_KEY_UP);
	key_table_insert(&window->keymap, XK_Escape, MML_KEY_ESCAPE);
	key_table_insert(&window->keymap, XK_Return, MML_KEY_RETURN);
	key_table_insert(&window->keymap, XK_BackSpace, MML_KEY_BACKSPACE);
	key_table_insert(&window->keymap, XK_Linefeed, MML_KEY_RETURN);
	key_table_insert(&window->keymap, XK_Pause, MML_KEY_PAUSE);
	key_table_insert(&window->keymap, XK_Scroll_Lock, MML_KEY_SCROLL_LOCK);
	key_table_insert(&window->keymap, XK_Tab, MML_KEY_TAB);
	key_table_insert(&window->keymap, XK_Delete, MML_KEY_DELETE);
	key_table_insert(&window->keymap, XK_Home, MML_KEY_HOME);
	key_table_insert(&window->keymap, XK_End, MML_KEY_END);
	key_table_insert(&window->keymap, XK_Page_Up, MML_KEY_PAGE_UP);
	key_table_insert(&window->keymap, XK_Page_Down, MML_KEY_PAGE_DOWN);
	key_table_insert(&window->keymap, XK_Insert, MML_KEY_INSERT);
	key_table_insert(&window->keymap, XK_Shift_L, MML_KEY_SHIFT);
	key_table_insert(&window->keymap, XK_Shift_R, MML_KEY_SHIFT);
	key_table_insert(&window->keymap, XK_Control_L, MML_KEY_CONTROL);
	key_table_insert(&window->keymap, XK_Control_R, MML_KEY_CONTROL);
	key_table_insert(&window->keymap, XK_Super_L, MML_KEY_SUPER);
	key_table_insert(&window->keymap, XK_Super_R, MML_KEY_SUPER);
	key_table_insert(&window->keymap, XK_Alt_L, MML_KEY_ALT);
	key_table_insert(&window->keymap, XK_Alt_R, MML_KEY_ALT);
	key_table_insert(&window->keymap, XK_space, MML_KEY_SPACE);
	key_table_insert(&window->keymap, XK_period, MML_KEY_PERIOD);
	key_table_insert(&window->keymap, XK_0, MML_KEY_0);
	key_table_insert(&window->keymap, XK_1, MML_KEY_1);
	key_table_insert(&window->keymap, XK_2, MML_KEY_2);
	key_table_insert(&window->keymap, XK_3, MML_KEY_3);
	key_table_insert(&window->keymap, XK_4, MML_KEY_4);
	key_table_insert(&window->keymap, XK_5, MML_KEY_5);
	key_table_insert(&window->keymap, XK_6, MML_KEY_6);
	key_table_insert(&window->keymap, XK_7, MML_KEY_7);
	key_table_insert(&window->keymap, XK_8, MML_KEY_8);
	key_table_insert(&window->keymap, XK_9, MML_KEY_9);
	key_table_insert(&window->keymap, XK_KP_0, MML_KEY_NP_0);
	key_table_insert(&window->keymap, XK_KP_1, MML_KEY_NP_1);
	key_table_insert(&window->keymap, XK_KP_2, MML_KEY_NP_2);
	key_table_insert(&window->keymap, XK_KP_3, MML_KEY_NP_3);
	key_table_insert(&window->keymap, XK_KP_4, MML_KEY_NP_4);
	key_table_insert(&window->keymap, XK_KP_5, MML_KEY_NP_5);
	key_table_insert(&window->keymap, XK_KP_6, MML_KEY_NP_6);
	key_table_insert(&window->keymap, XK_KP_7, MML_KEY_NP_7);
	key_table_insert(&window->keymap, XK_KP_8, MML_KEY_NP_8);
	key_table_insert(&window->keymap, XK_KP_9, MML_KEY_NP_9);
	key_table_insert(&window->keymap, XK_Num_Lock, MML_KEY_NUM_LOCK);
	key_table_insert(&window->keymap, XK_KP_Multiply, MML_KEY_NP_MULTIPLY);
	key_table_insert(&window->keymap, XK_KP_Add, MML_KEY_NP_ADD);
	key_table_insert(&window->keymap, XK_KP_Divide, MML_KEY_NP_DIVIDE);
	key_table_insert(&window->keymap, XK_KP_Subtract, MML_KEY_NP_SUBTRACT);
	key_table_insert(&window->keymap, XK_KP_Decimal, MML_KEY_NP_DECIMAL);
	key_table_insert(&window->keymap, XK_KP_Delete, MML_KEY_NP_DELETE);
	key_table_insert(&window->keymap, XK_KP_Enter, MML_KEY_NP_ENTER);
	key_table_insert(&window->keymap, XK_apostrophe, MML_KEY_APOSTROPHE);
	key_table_insert(&window->keymap, XK_comma, MML_KEY_COMMA);
	key_table_insert(&window->keymap, XK_minus, MML_KEY_MINUS);
	key_table_insert(&window->keymap, XK_slash, MML_KEY_SLASH);
	key_table_insert(&window->keymap, XK_semicolon, MML_KEY_SEMICOLON);
	key_table_insert(&window->keymap, XK_equal, MML_KEY_EQUAL);
	key_table_insert(&window->keymap, XK_0, MML_KEY_LEFT_BRACKET);
	key_table_insert(&window->keymap, XK_9, MML_KEY_RIGHT_BRACKET);
	key_table_insert(&window->keymap, XK_backslash, MML_KEY_BACKSLASH);
	key_table_insert(&window->keymap, XK_grave, MML_KEY_GRAVE_ACCENT);
	key_table_insert(&window->keymap, XK_Caps_Lock, MML_KEY_CAPS_LOCK);

	return window;
}

void mml_free_window(struct mml_window* window) {
	mml_free_bitmap(window->backbuffer);

	glXDestroyContext(window->display, window->context);

	XFree(window->visual);
	XFreeColormap(window->display, window->attribs.colormap);
	XDestroyWindow(window->display, window->window);
	XCloseDisplay(window->display);

	free(window);
}

int mml_next_event(struct mml_window* window, struct mml_event* e) {
	int r;
	XEvent event;
	XWindowAttributes attribs;
	KeySym sym;

	e->type = -1;

	r = XPending(window->display);

	if (r) {
		XNextEvent(window->display, &event);
		switch (event.type) {
			case ClientMessage:
				e->type = MML_QUIT;
				break;
			case Expose:
				XGetWindowAttributes(window->display, window->window, &attribs);

				if (attribs.width != window->w || attribs.height != window->h) {
					glViewport(0, 0, attribs.width, attribs.height);
					window->w = attribs.width / window->pixel_size;
					window->h = attribs.height / window->pixel_size;

					e->type = MML_WINDOW_RESIZE;
					e->as.window_resize_event.w = window->w;
					e->as.window_resize_event.h = window->h;

					if (!(window->flags & MML_WINDOW_STRETCH)) {
						mml_free_bitmap(window->backbuffer);
						window->backbuffer = mml_new_bitmap(window->w, window->h);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window->w, window->h, 0, GL_RGBA,
								GL_UNSIGNED_BYTE, window->backbuffer->pixels);
					}
				}
				break;
			case KeyPress:
				sym = XLookupKeysym(&event.xkey, 0);
				e->type = MML_KEY_PRESS;
				e->as.keyboard_event.key = search_key_table(&window->keymap, sym);
				break;
			case KeyRelease:
				sym = XLookupKeysym(&event.xkey, 0);

				e->type = MML_KEY_RELEASE;
				e->as.keyboard_event.key = search_key_table(&window->keymap, sym);
				break;
			case MotionNotify:
				e->type = MML_MOUSE_MOVE;
				e->as.mouse_move_event.x = event.xmotion.x / window->pixel_size;
				e->as.mouse_move_event.y = event.xmotion.y / window->pixel_size;
				break;
			case ButtonPress:
				e->type = MML_MOUSE_PRESS;
				e->as.mouse_button_event.button = event.xbutton.button - 1;
				break;
			case ButtonRelease:
				e->type = MML_MOUSE_RELEASE;
				e->as.mouse_button_event.button = event.xbutton.button - 1;
				break;
			default: break;
		}
	}

	return r;
}

void mml_update_window(struct mml_window* window) {
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, window->backbuffer->w, window->backbuffer->h, GL_RGBA,
			GL_UNSIGNED_BYTE, window->backbuffer->pixels);

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(-1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(-1.0f, 1.0f);
	glEnd();

	glXSwapBuffers(window->display, window->window);
}

void mml_query_window(struct mml_window* window, int* flags, int* width, int* height, int* pixel_size) {
	if (flags) {
		*flags = window->flags;
	}

	if (width) {
		*width = window->w;
	}

	if (height) {
		*height = window->h;
	}

	if (pixel_size) {
		*pixel_size = window->pixel_size;
	}
}

struct mml_bitmap* mml_get_backbuffer(struct mml_window* window) {
	return window->backbuffer;
}
