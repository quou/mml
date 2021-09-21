#include <stdio.h>

#include <windows.h>
#include <GL/gl.h>
#include <sys/stat.h>

#include "mml.h"
#include "keytable.h"

unsigned long long mml_freq;
int mml_haspc;

struct mml_window {
	int w, h;

	int flags;

	HWND hwnd;
	HDC device_context;
	HGLRC render_context;

	int pixel_size;
	unsigned int gl_tex;
	struct mml_bitmap* backbuffer;
	struct mml_key_table keymap;

	struct mml_event last_event;
};

void mml_init_time() {
	if (QueryPerformanceFrequency((LARGE_INTEGER*)&mml_freq)) {
		mml_haspc = 1;
	} else {
		mml_freq = 1000;
		mml_haspc = 0;
	}
}

unsigned long long mml_get_time() {
	unsigned long long now;

	if (mml_haspc) {
		QueryPerformanceCounter((LARGE_INTEGER*)&now);
	} else {
		now = (unsigned long long)timeGetTime();
	}

	return now;
}

unsigned long long mml_get_frequency() {
	return mml_freq;
}

static LRESULT CALLBACK win32_event_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	LONG_PTR lpUserData;
	struct mml_window* window;
	int nw, nh;

	lpUserData = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	window = (struct mml_window*)lpUserData;
	if (!window) { goto no_window; }

	switch (msg) {
	case WM_DESTROY:
		window->last_event.type = MML_QUIT;
		return 0;
	case WM_SIZE:
		nw = lparam & 0xFFFF;
		nh = (lparam >> 16) & 0xFFFF;

		glViewport(0, 0, nw, nh);

		window->w = nw / window->pixel_size;
		window->h = nh / window->pixel_size;

		window->last_event.type = MML_WINDOW_RESIZE;
		window->last_event.as.window_resize_event.w = window->w;
		window->last_event.as.window_resize_event.h = window->h;

		if (!(window->flags & MML_WINDOW_STRETCH)) {
			mml_free_bitmap(window->backbuffer);
			window->backbuffer = mml_new_bitmap(window->w, window->h);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window->w, window->h, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, window->backbuffer->pixels);
		}
		return 0;
	case WM_KEYDOWN:
		window->last_event.type = MML_KEY_PRESS;
		window->last_event.as.keyboard_event.key = search_key_table(&window->keymap, (int)wparam);
		return 0;
	case WM_KEYUP:
		window->last_event.type = MML_KEY_RELEASE;
		window->last_event.as.keyboard_event.key = search_key_table(&window->keymap, (int)wparam);
		return 0;
	case WM_MOUSEMOVE:
		window->last_event.type = MML_MOUSE_MOVE;
		window->last_event.as.mouse_move_event.x = lparam & 0xFFFF		/ window->pixel_size;
		window->last_event.as.mouse_move_event.y = (lparam >> 16) & 0xFFFF	/ window->pixel_size;
		return 0;
	case WM_LBUTTONDOWN:
		window->last_event.type = MML_MOUSE_PRESS;
		window->last_event.as.mouse_button_event.button = MML_MOUSE_LEFT;
		return 0;
	case WM_MBUTTONDOWN:
		window->last_event.type = MML_MOUSE_PRESS;
		window->last_event.as.mouse_button_event.button = MML_MOUSE_MIDDLE;
		return 0;
	case WM_RBUTTONDOWN:
		window->last_event.type = MML_MOUSE_PRESS;
		window->last_event.as.mouse_button_event.button = MML_MOUSE_RIGHT;
		return 0;
	case WM_LBUTTONUP:
		window->last_event.type = MML_MOUSE_RELEASE;
		window->last_event.as.mouse_button_event.button = MML_MOUSE_LEFT;
		return 0;
	case WM_MBUTTONUP:
		window->last_event.type = MML_MOUSE_RELEASE;
		window->last_event.as.mouse_button_event.button = MML_MOUSE_MIDDLE;
		return 0;
	case WM_RBUTTONUP:
		window->last_event.type = MML_MOUSE_RELEASE;
		window->last_event.as.mouse_button_event.button = MML_MOUSE_RIGHT;
		return 0;
	default: break;
	}

no_window:
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

struct mml_window* mml_new_window(const char* name, int w, int h, int pixel_size, int flags) {
	struct mml_window* window;
	WNDCLASS wc;
	DWORD dwExStyle, dwStyle;
	RECT rWndRect = { 0, 0, w, h };
	int pf, create_width, create_height;
	HMONITOR hmon;
	MONITORINFO mi;
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	window = malloc(sizeof(struct mml_window));

	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = win32_event_callback;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpszMenuName = NULL;
	wc.hbrBackground = NULL;
	wc.lpszClassName = L"mml";
	RegisterClass(&wc);

	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	dwStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;

	if (flags & MML_WINDOW_RESIZABLE) {
		dwStyle |= WS_THICKFRAME | WS_MAXIMIZEBOX;
	}

	if (flags & MML_WINDOW_FULLSCREEN) {
		dwExStyle = 0;
		dwStyle = WS_VISIBLE | WS_POPUP;
		hmon = MonitorFromWindow(NULL, MONITOR_DEFAULTTONEAREST);
		mi = (MONITORINFO) { sizeof(mi) };
		if (GetMonitorInfo(hmon, &mi)) {
			rWndRect = (RECT){ 0, 0, mi.rcMonitor.right, mi.rcMonitor.bottom };
		}
	}

	AdjustWindowRectEx(&rWndRect, dwStyle, FALSE, dwExStyle);
	create_width = rWndRect.right - rWndRect.left;
	create_height = rWndRect.bottom - rWndRect.top;

	window->hwnd = CreateWindowEx(dwExStyle, L"mml", L"", dwStyle, 0, 0, create_width, create_height, NULL, NULL, GetModuleHandle(NULL), window);
	SetWindowLongPtr(window->hwnd, GWLP_USERDATA, (LONG_PTR)window);

	window->device_context = GetDC((HWND)window->hwnd);

	if (!(pf = ChoosePixelFormat(window->device_context, &pfd))) {
		printf("Error choosing pixel format\n");
		free(window);
		return NULL;
	}
	SetPixelFormat(window->device_context, pf, &pfd);

	if (!(window->render_context = wglCreateContext(window->device_context))) {
		printf("Error creating OpenGL context\n");
		free(window);
		return NULL;
	}
	wglMakeCurrent(window->device_context, window->render_context);

	if (name) {
		SetWindowTextA(window->hwnd, name);
	}

	/* Set the keyboard mapping */
	init_key_table(&window->keymap);
	key_table_insert(&window->keymap, 0x00, MML_KEY_UNKNOWN);
	key_table_insert(&window->keymap, 0x41, MML_KEY_A);
	key_table_insert(&window->keymap, 0x42, MML_KEY_B);
	key_table_insert(&window->keymap, 0x43, MML_KEY_C);
	key_table_insert(&window->keymap, 0x44, MML_KEY_D);
	key_table_insert(&window->keymap, 0x45, MML_KEY_E);
	key_table_insert(&window->keymap, 0x46, MML_KEY_F);
	key_table_insert(&window->keymap, 0x47, MML_KEY_G);
	key_table_insert(&window->keymap, 0x48, MML_KEY_H);
	key_table_insert(&window->keymap, 0x49, MML_KEY_I);
	key_table_insert(&window->keymap, 0x4A, MML_KEY_J);
	key_table_insert(&window->keymap, 0x4B, MML_KEY_K);
	key_table_insert(&window->keymap, 0x4C, MML_KEY_L);
	key_table_insert(&window->keymap, 0x4D, MML_KEY_M);
	key_table_insert(&window->keymap, 0x4E, MML_KEY_N);
	key_table_insert(&window->keymap, 0x4F, MML_KEY_O);
	key_table_insert(&window->keymap, 0x50, MML_KEY_P);
	key_table_insert(&window->keymap, 0x51, MML_KEY_Q);
	key_table_insert(&window->keymap, 0x52, MML_KEY_R);
	key_table_insert(&window->keymap, 0x53, MML_KEY_S);
	key_table_insert(&window->keymap, 0x54, MML_KEY_T);
	key_table_insert(&window->keymap, 0x55, MML_KEY_U);
	key_table_insert(&window->keymap, 0x56, MML_KEY_V);
	key_table_insert(&window->keymap, 0x57, MML_KEY_W);
	key_table_insert(&window->keymap, 0x58, MML_KEY_X);
	key_table_insert(&window->keymap, 0x59, MML_KEY_Y);
	key_table_insert(&window->keymap, 0x5A, MML_KEY_Z);
	key_table_insert(&window->keymap, 0x30, MML_KEY_0);
	key_table_insert(&window->keymap, 0x31, MML_KEY_1);
	key_table_insert(&window->keymap, 0x32, MML_KEY_2);
	key_table_insert(&window->keymap, 0x33, MML_KEY_3);
	key_table_insert(&window->keymap, 0x34, MML_KEY_4);
	key_table_insert(&window->keymap, 0x35, MML_KEY_5);
	key_table_insert(&window->keymap, 0x36, MML_KEY_6);
	key_table_insert(&window->keymap, 0x37, MML_KEY_7);
	key_table_insert(&window->keymap, 0x38, MML_KEY_8);
	key_table_insert(&window->keymap, 0x39, MML_KEY_9);
	key_table_insert(&window->keymap, VK_F1, MML_KEY_F1);
	key_table_insert(&window->keymap, VK_F2, MML_KEY_F2);
	key_table_insert(&window->keymap, VK_F3, MML_KEY_F3);
	key_table_insert(&window->keymap, VK_F4, MML_KEY_F4);
	key_table_insert(&window->keymap, VK_F5, MML_KEY_F5);
	key_table_insert(&window->keymap, VK_F6, MML_KEY_F6);
	key_table_insert(&window->keymap, VK_F7, MML_KEY_F7);
	key_table_insert(&window->keymap, VK_F8, MML_KEY_F8);
	key_table_insert(&window->keymap, VK_F9, MML_KEY_F9);
	key_table_insert(&window->keymap, VK_F10, MML_KEY_F10);
	key_table_insert(&window->keymap, VK_F11, MML_KEY_F11);
	key_table_insert(&window->keymap, VK_F12, MML_KEY_F12);
	key_table_insert(&window->keymap, VK_DOWN, MML_KEY_DOWN);
	key_table_insert(&window->keymap, VK_LEFT, MML_KEY_LEFT);
	key_table_insert(&window->keymap, VK_RIGHT, MML_KEY_RIGHT);
	key_table_insert(&window->keymap, VK_UP, MML_KEY_UP);
	key_table_insert(&window->keymap, VK_ESCAPE, MML_KEY_ESCAPE);
	key_table_insert(&window->keymap, VK_RETURN, MML_KEY_RETURN);
	key_table_insert(&window->keymap, VK_BACK, MML_KEY_BACKSPACE);
	key_table_insert(&window->keymap, VK_PAUSE, MML_KEY_PAUSE);
	key_table_insert(&window->keymap, VK_SCROLL, MML_KEY_SCROLL_LOCK);
	key_table_insert(&window->keymap, VK_TAB, MML_KEY_TAB);
	key_table_insert(&window->keymap, VK_DELETE, MML_KEY_DELETE);
	key_table_insert(&window->keymap, VK_HOME, MML_KEY_HOME);
	key_table_insert(&window->keymap, VK_END, MML_KEY_END);
	key_table_insert(&window->keymap, VK_PRIOR, MML_KEY_PAGE_UP);
	key_table_insert(&window->keymap, VK_NEXT, MML_KEY_PAGE_DOWN);
	key_table_insert(&window->keymap, VK_INSERT, MML_KEY_INSERT);
	key_table_insert(&window->keymap, VK_LSHIFT, MML_KEY_SHIFT);
	key_table_insert(&window->keymap, VK_RSHIFT, MML_KEY_SHIFT);
	key_table_insert(&window->keymap, VK_LCONTROL, MML_KEY_CONTROL);
	key_table_insert(&window->keymap, VK_RCONTROL, MML_KEY_CONTROL);
	key_table_insert(&window->keymap, VK_LWIN, MML_KEY_SUPER);
	key_table_insert(&window->keymap, VK_RWIN, MML_KEY_SUPER);
	key_table_insert(&window->keymap, VK_MENU, MML_KEY_ALT);
	key_table_insert(&window->keymap, VK_SPACE, MML_KEY_SPACE);
	key_table_insert(&window->keymap, VK_OEM_PERIOD, MML_KEY_PERIOD);
	key_table_insert(&window->keymap, VK_NUMPAD0, MML_KEY_NP_0);
	key_table_insert(&window->keymap, VK_NUMPAD1, MML_KEY_NP_1);
	key_table_insert(&window->keymap, VK_NUMPAD2, MML_KEY_NP_2);
	key_table_insert(&window->keymap, VK_NUMPAD3, MML_KEY_NP_3);
	key_table_insert(&window->keymap, VK_NUMPAD4, MML_KEY_NP_4);
	key_table_insert(&window->keymap, VK_NUMPAD5, MML_KEY_NP_5);
	key_table_insert(&window->keymap, VK_NUMPAD6, MML_KEY_NP_6);
	key_table_insert(&window->keymap, VK_NUMPAD7, MML_KEY_NP_7);
	key_table_insert(&window->keymap, VK_NUMPAD8, MML_KEY_NP_8);
	key_table_insert(&window->keymap, VK_NUMPAD9, MML_KEY_NP_9);
	key_table_insert(&window->keymap, VK_NUMLOCK, MML_KEY_NUM_LOCK);
	key_table_insert(&window->keymap, VK_MULTIPLY, MML_KEY_NP_MULTIPLY);
	key_table_insert(&window->keymap, VK_ADD, MML_KEY_NP_ADD);
	key_table_insert(&window->keymap, VK_DIVIDE, MML_KEY_NP_DIVIDE);
	key_table_insert(&window->keymap, VK_SUBTRACT, MML_KEY_NP_SUBTRACT);
	key_table_insert(&window->keymap, VK_DECIMAL, MML_KEY_NP_DECIMAL);
	key_table_insert(&window->keymap, VK_DELETE, MML_KEY_NP_DELETE);
	key_table_insert(&window->keymap, VK_RETURN, MML_KEY_NP_ENTER);
	key_table_insert(&window->keymap, VK_OEM_7, MML_KEY_APOSTROPHE);
	key_table_insert(&window->keymap, VK_OEM_COMMA, MML_KEY_COMMA);
	key_table_insert(&window->keymap, VK_OEM_MINUS, MML_KEY_MINUS);
	key_table_insert(&window->keymap, VK_OEM_2, MML_KEY_SLASH);
	key_table_insert(&window->keymap, VK_OEM_1, MML_KEY_SEMICOLON);
	key_table_insert(&window->keymap, VK_OEM_NEC_EQUAL, MML_KEY_EQUAL);
	key_table_insert(&window->keymap, VK_OEM_5, MML_KEY_BACKSLASH);
	key_table_insert(&window->keymap, VK_OEM_3, MML_KEY_GRAVE_ACCENT);
	key_table_insert(&window->keymap, VK_CAPITAL, MML_KEY_CAPS_LOCK);

	create_width /= pixel_size;
	create_height /= pixel_size;

	window->w = create_width;
	window->h = create_height;

	if (flags & MML_WINDOW_STRETCH) {
		create_width = w;
		create_height = h;
	}

	window->flags = flags;
	window->pixel_size = pixel_size;

	window->backbuffer = mml_new_bitmap(create_width, create_height);

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &window->gl_tex);
	glBindTexture(GL_TEXTURE_2D, window->gl_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, create_width, create_height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, window->backbuffer->pixels);

	return window;
}

void mml_free_window(struct mml_window* window) {
	glDeleteTextures(1, &window->gl_tex);

	PostQuitMessage(0);
	DestroyWindow(window->hwnd);
	wglDeleteContext(window->render_context);

	mml_free_bitmap(window->backbuffer);

	free(window);
}

int mml_next_event(struct mml_window* window, struct mml_event* e) {
	int r;
	MSG msg;

	e->type = -1;
	window->last_event.type = -1;

	if ((r = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		*e = window->last_event;
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

	SwapBuffers(window->device_context);
}

struct mml_bitmap* mml_get_backbuffer(struct mml_window* window) {
	return window->backbuffer;
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

int mml_list_directory(struct mml_directory_list* list, const char* path) {
	int len;
	char* windows_path;
	int i, capacity;
	WIN32_FIND_DATAA data;
	HANDLE hFind;

	len = (int)strlen(path);
	windows_path = calloc(len + 32, 1);
	strcpy(windows_path, path);
	windows_path[len] = '\0';
	strcat(windows_path, "\\*.*");

	for (i = 0; i < len; i++) {
		if (windows_path[i] == '/') {
			windows_path[i] = '\\';
		}
	}

	list->entries = NULL;
	list->count = 0;

	hFind = FindFirstFileA(windows_path, &data);
	if (hFind == INVALID_HANDLE_VALUE) {
		return MML_FILE_NOT_FOUND;
	}

	capacity = 0;
	do {
		if (strcmp(data.cFileName, ".") == 0) { continue; }
		if (strcmp(data.cFileName, "..") == 0) { continue; }
		
		if (list->count >= capacity) {
			capacity = capacity < 8 ? 8 : capacity * 2;
			list->entries = realloc(list->entries, capacity * sizeof(char*));
		}

		len = (int)strlen(data.cFileName);
		list->entries[list->count] = malloc(len + 1);
		strcpy(list->entries[list->count], data.cFileName);
		list->entries[list->count][len] = '\0';
		list->count++;
	} while (FindNextFileA(hFind, &data));
	FindClose(hFind);

	free(windows_path);

	return MML_OK;
}

void mml_deinit_directory_list(struct mml_directory_list* list) {
	int i;

	for (i = 0; i < list->count; i++) {
		free(list->entries[i]);
	}

	if (list->count > 0) {
		free(list->entries);
	}

	list->entries = NULL;
	list->count = 0;
}

int mml_stat(struct mml_stat* ostat, const char* filename) {
	struct stat path_stat;

	if (stat(filename, &path_stat) == -1) {
		return MML_FILE_NOT_FOUND;
	}

	if      (path_stat.st_mode & _S_IFDIR) { ostat->type = MML_FILE_DIR; }
	else if (path_stat.st_mode & _S_IFREG) { ostat->type = MML_FILE_NORMAL; }
	else                                   { ostat->type = MML_FILE_UNKNOWN; }

	ostat->mod_time = path_stat.st_mtime;
	ostat->access_time = path_stat.st_atime;
	ostat->create_time = path_stat.st_ctime;

	return MML_OK;
}

int mml_mkdir(const char* filename) {
	CreateDirectoryA(filename, NULL);
	return MML_OK;
}

int mml_mkfile(const char* filename) {
	CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	return MML_OK;
}
