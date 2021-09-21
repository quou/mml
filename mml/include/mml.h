#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum {
	MML_OK,
	MML_FAILURE_UNKNOWN_REASON,
	MML_FILE_NOT_FOUND,
	MML_FILE_EXISTS,
	MML_INVALID_DATA,
	MML_TOO_MANY_THREADS,
	MML_THREAD_CREATE_ERROR,
	MML_THREAD_JOIN_ERROR
};

struct mml_window;
struct mml_bitmap;
struct mml_font;

struct mml_rect {
	int x, y, w, h;
};

static struct mml_rect mml_rect(int x, int y, int w, int h) {
	return (struct mml_rect) { x, y, w, h };
}

void mml_init();
void mml_deinit();

enum {
	MML_KEY_UNKNOWN = 0,
	MML_KEY_SPACE,
	MML_KEY_APOSTROPHE,
	MML_KEY_COMMA,
	MML_KEY_MINUS,
	MML_KEY_PERIOD,
	MML_KEY_SLASH,
	MML_KEY_0,
	MML_KEY_1,
	MML_KEY_2,
	MML_KEY_3,
	MML_KEY_4,
	MML_KEY_5,
	MML_KEY_6,
	MML_KEY_7,
	MML_KEY_8,
	MML_KEY_9,
	MML_KEY_SEMICOLON,
	MML_KEY_EQUAL,
	MML_KEY_A,
	MML_KEY_B,
	MML_KEY_C,
	MML_KEY_D,
	MML_KEY_E,
	MML_KEY_F,
	MML_KEY_G,
	MML_KEY_H,
	MML_KEY_I,
	MML_KEY_J,
	MML_KEY_K,
	MML_KEY_L,
	MML_KEY_M,
	MML_KEY_N,
	MML_KEY_O,
	MML_KEY_P,
	MML_KEY_Q,
	MML_KEY_R,
	MML_KEY_S,
	MML_KEY_T,
	MML_KEY_U,
	MML_KEY_V,
	MML_KEY_W,
	MML_KEY_X,
	MML_KEY_Y,
	MML_KEY_Z,
	MML_KEY_LEFT_BRACKET,
	MML_KEY_BACKSLASH,
	MML_KEY_RIGHT_BRACKET,
	MML_KEY_GRAVE_ACCENT,
	MML_KEY_ESCAPE,
	MML_KEY_RETURN,
	MML_KEY_TAB,
	MML_KEY_BACKSPACE,
	MML_KEY_INSERT,
	MML_KEY_DELETE,
	MML_KEY_RIGHT,
	MML_KEY_LEFT,
	MML_KEY_DOWN,
	MML_KEY_UP,
	MML_KEY_PAGE_UP,
	MML_KEY_PAGE_DOWN,
	MML_KEY_HOME,
	MML_KEY_END,
	MML_KEY_CAPS_LOCK,
	MML_KEY_SCROLL_LOCK,
	MML_KEY_NUM_LOCK,
	MML_KEY_PRINT_SCREEN,
	MML_KEY_PAUSE,
	MML_KEY_F1,
	MML_KEY_F2,
	MML_KEY_F3,
	MML_KEY_F4,
	MML_KEY_F5,
	MML_KEY_F6,
	MML_KEY_F7,
	MML_KEY_F8,
	MML_KEY_F9,
	MML_KEY_F10,
	MML_KEY_F11,
	MML_KEY_F12,
	MML_KEY_F13,
	MML_KEY_F14,
	MML_KEY_F15,
	MML_KEY_F16,
	MML_KEY_F17,
	MML_KEY_F18,
	MML_KEY_F19,
	MML_KEY_F20,
	MML_KEY_F21,
	MML_KEY_F22,
	MML_KEY_F23,
	MML_KEY_F24,
	MML_KEY_F25,
	MML_KEY_NP_0,
	MML_KEY_NP_1,
	MML_KEY_NP_2,
	MML_KEY_NP_3,
	MML_KEY_NP_4,
	MML_KEY_NP_5,
	MML_KEY_NP_6,
	MML_KEY_NP_7,
	MML_KEY_NP_8,
	MML_KEY_NP_9,
	MML_KEY_NP_DECIMAL,
	MML_KEY_NP_DIVIDE,
	MML_KEY_NP_MULTIPLY,
	MML_KEY_NP_SUBTRACT,
	MML_KEY_NP_ADD,
	MML_KEY_NP_ENTER,
	MML_KEY_NP_EQUAL,
	MML_KEY_NP_DELETE,
	MML_KEY_SHIFT,
	MML_KEY_CONTROL,
	MML_KEY_ALT,
	MML_KEY_SUPER,
	MML_KEY_MENU,

	/* A little extra space is allocated
	 * for keys that may have multiple binds */
	MML_KEY_COUNT = MML_KEY_MENU + 32
};

enum {
	MML_MOUSE_LEFT = 0,
	MML_MOUSE_MIDDLE,
	MML_MOUSE_RIGHT,
	MML_MOUSE_COUNT
};

enum {
	MML_QUIT = 0,
	MML_KEY_PRESS,
	MML_KEY_RELEASE,
	MML_WINDOW_RESIZE,
	MML_MOUSE_MOVE,
	MML_MOUSE_PRESS,
	MML_MOUSE_RELEASE
};

struct mml_keyboard_event {
	int key;
};

struct mml_mouse_button_event {
	int button;
};

struct mml_mouse_move_event {
	int x, y;
};

struct mml_window_resize_event {
	int w, h;
};

struct mml_event {
	int type;
	union {
		struct mml_keyboard_event keyboard_event;
		struct mml_mouse_move_event mouse_move_event;
		struct mml_mouse_button_event mouse_button_event;
		struct mml_window_resize_event window_resize_event;
	} as;
};

void mml_init_time();
unsigned long long mml_get_time();
unsigned long long mml_get_frequency();

enum {
	MML_WINDOW_FULLSCREEN = 1 << 0,
	MML_WINDOW_RESIZABLE = 1 << 1,
	MML_WINDOW_STRETCH = 1 << 2,
};

struct mml_window* mml_new_window(const char* name, int w, int h, int pixel_size, int flags);
void mml_free_window(struct mml_window* window);
int mml_next_event(struct mml_window* window, struct mml_event* e);
void mml_update_window(struct mml_window* window);
struct mml_bitmap* mml_get_backbuffer(struct mml_window* window);

void mml_query_window(struct mml_window* window, int* flags, int* width, int* height, int* pixel_size);

struct mml_pixel {
	unsigned char r, g, b, a;
};

static struct mml_pixel mml_pixel(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	return (struct mml_pixel) { r, g, b, a };
}

static struct mml_pixel mml_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	return (struct mml_pixel) { r, g, b, a };
}

struct mml_bitmap {
	struct mml_pixel* pixels;
	int w, h;
};

struct mml_bitmap* mml_new_bitmap(int w, int h);
struct mml_bitmap* mml_load_bitmap(const char* filename);
void mml_free_bitmap(struct mml_bitmap* bitmap);
void mml_render_fill(struct mml_bitmap* dst, struct mml_pixel color);
void mml_render_copy(struct mml_bitmap* dst, int x, int y, struct mml_bitmap* src, struct mml_rect src_rect);
void mml_render_copy_ex(struct mml_bitmap* dst, int x, int y, struct mml_bitmap* src, struct mml_rect src_rect,
		struct mml_pixel color, int scale);
void mml_fill_rect(struct mml_bitmap* dst, struct mml_rect rect, struct mml_pixel color);
int mml_render_text(struct mml_bitmap* dst, struct mml_font* font,
		const char* text, int x, int y, struct mml_pixel color);

struct mml_font* mml_load_font(const char* filename, float size);
struct mml_font* mml_load_font_from_memory(void* data, int filesize, float size);
void mml_free_font(struct mml_font* font);

void mml_set_font_tab_size(struct mml_font* font, int n);
int mml_get_font_tab_size(struct mml_font* font);

int mml_text_width(struct mml_font* font, const char* text);
int mml_text_height(struct mml_font* font);

int mml_read_file(const char* path, void** buffer, unsigned int* size, int terminate);

struct mml_directory_list {
	char** entries;
	int count;
};

int mml_list_directory(struct mml_directory_list* list, const char* path);
void mml_deinit_directory_list(struct mml_directory_list* list);

enum {
	MML_FILE_NORMAL,
	MML_FILE_DIR,
	MML_FILE_UNKNOWN
};

struct mml_stat {
	int type;
	unsigned long long mod_time;
	unsigned long long create_time;
	unsigned long long access_time;
};

int mml_stat(struct mml_stat* ostat, const char* filename);
int mml_mkdir(const char* filename);
int mml_mkfile(const char* filename);

#ifdef __cplusplus
}
#endif
