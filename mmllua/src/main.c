#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <mml.h>

#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>

struct l_bitmap {
	int managed;
	struct mml_bitmap* self;
};

static const char* key_str[] = {
	[ MML_KEY_UNKNOWN ] = "unknown",
	[ MML_KEY_SPACE ] = "space",
	[ MML_KEY_APOSTROPHE ] = "'",
	[ MML_KEY_COMMA ] = ",",
	[ MML_KEY_MINUS ] = "-",
	[ MML_KEY_PERIOD ] = ".",
	[ MML_KEY_SLASH ] = "/",
	[ MML_KEY_0 ] = "0",
	[ MML_KEY_1 ] = "1",
	[ MML_KEY_2 ] = "2",
	[ MML_KEY_3 ] = "3",
	[ MML_KEY_4 ] = "4",
	[ MML_KEY_5 ] = "5",
	[ MML_KEY_6 ] = "6",
	[ MML_KEY_7 ] = "7",
	[ MML_KEY_8 ] = "8",
	[ MML_KEY_9 ] = "9",
	[ MML_KEY_SEMICOLON ] = ";",
	[ MML_KEY_EQUAL ] = "=",
	[ MML_KEY_A ] = "a",
	[ MML_KEY_B ] = "b",
	[ MML_KEY_C ] = "c",
	[ MML_KEY_D ] = "d",
	[ MML_KEY_E ] = "e",
	[ MML_KEY_F ] = "f",
	[ MML_KEY_G ] = "g",
	[ MML_KEY_H ] = "h",
	[ MML_KEY_I ] = "i",
	[ MML_KEY_J ] = "j",
	[ MML_KEY_K ] = "k",
	[ MML_KEY_L ] = "l",
	[ MML_KEY_M ] = "m",
	[ MML_KEY_N ] = "n",
	[ MML_KEY_O ] = "o",
	[ MML_KEY_P ] = "p",
	[ MML_KEY_Q ] = "q",
	[ MML_KEY_R ] = "r",
	[ MML_KEY_S ] = "s",
	[ MML_KEY_T ] = "t",
	[ MML_KEY_U ] = "u",
	[ MML_KEY_V ] = "v",
	[ MML_KEY_W ] = "w",
	[ MML_KEY_X ] = "k",
	[ MML_KEY_Y ] = "y",
	[ MML_KEY_Z ] = "z",
	[ MML_KEY_LEFT_BRACKET ] = "[",
	[ MML_KEY_BACKSLASH ] = "\\",
	[ MML_KEY_RIGHT_BRACKET ] = "]",
	[ MML_KEY_GRAVE_ACCENT ] = "`",
	[ MML_KEY_ESCAPE ] = "escape",
	[ MML_KEY_RETURN ] = "return",
	[ MML_KEY_TAB ] = "tab",
	[ MML_KEY_BACKSPACE ] = "backspace",
	[ MML_KEY_INSERT ] = "insert",
	[ MML_KEY_DELETE ] = "delete",
	[ MML_KEY_RIGHT ] = "right",
	[ MML_KEY_LEFT ] = "left",
	[ MML_KEY_DOWN ] = "down",
	[ MML_KEY_UP ] = "up",
	[ MML_KEY_PAGE_UP ] = "page up",
	[ MML_KEY_PAGE_DOWN ] = "page down",
	[ MML_KEY_HOME ] = "home",
	[ MML_KEY_END ] = "end",
	[ MML_KEY_CAPS_LOCK ] = "caps lock",
	[ MML_KEY_SCROLL_LOCK ] = "scroll lock",
	[ MML_KEY_NUM_LOCK ] = "num lock",
	[ MML_KEY_PRINT_SCREEN ] = "print screen",
	[ MML_KEY_PAUSE ] = "pause",
	[ MML_KEY_F1 ] = "f1",
	[ MML_KEY_F2 ] = "f2",
	[ MML_KEY_F3 ] = "f3",
	[ MML_KEY_F4 ] = "f4",
	[ MML_KEY_F5 ] = "f5",
	[ MML_KEY_F6 ] = "f6",
	[ MML_KEY_F7 ] = "f7",
	[ MML_KEY_F8 ] = "f8",
	[ MML_KEY_F9 ] = "f9",
	[ MML_KEY_F10 ] = "f10",
	[ MML_KEY_F11 ] = "f11",
	[ MML_KEY_F12 ] = "f12",
	[ MML_KEY_F13 ] = "f13",
	[ MML_KEY_F14 ] = "f14",
	[ MML_KEY_F15 ] = "f15",
	[ MML_KEY_F16 ] = "f16",
	[ MML_KEY_F17 ] = "f17",
	[ MML_KEY_F18 ] = "f18",
	[ MML_KEY_F19 ] = "f19",
	[ MML_KEY_F20 ] = "f20",
	[ MML_KEY_F21 ] = "f21",
	[ MML_KEY_F22 ] = "f22",
	[ MML_KEY_F23 ] = "f23",
	[ MML_KEY_F24 ] = "f24",
	[ MML_KEY_F25 ] = "f25",
	[ MML_KEY_NP_0 ] = "np 0",
	[ MML_KEY_NP_1 ] = "np 1",
	[ MML_KEY_NP_2 ] = "np 2",
	[ MML_KEY_NP_3 ] = "np 3",
	[ MML_KEY_NP_4 ] = "np 4",
	[ MML_KEY_NP_5 ] = "np 5",
	[ MML_KEY_NP_6 ] = "np 6",
	[ MML_KEY_NP_7 ] = "np 7",
	[ MML_KEY_NP_8 ] = "np 8",
	[ MML_KEY_NP_9 ] = "np 9",
	[ MML_KEY_NP_DECIMAL ] = "np .",
	[ MML_KEY_NP_DIVIDE ] = "np /",
	[ MML_KEY_NP_MULTIPLY ] = "np *",
	[ MML_KEY_NP_SUBTRACT ] = "np -",
	[ MML_KEY_NP_ADD ] = "np +",
	[ MML_KEY_NP_ENTER ] = "np enter",
	[ MML_KEY_NP_EQUAL ] = "np =",
	[ MML_KEY_NP_DELETE ] = "np delete",
	[ MML_KEY_SHIFT ] = "shift",
	[ MML_KEY_CONTROL ] = "control",
	[ MML_KEY_ALT ] = "alt",
	[ MML_KEY_SUPER ] = "super",
	[ MML_KEY_MENU ] = "menu"
};

struct l_v2 {
	double x, y;
};

static struct l_v2 check_v2(lua_State* L, int i) {
	struct l_v2 r;
	if (!lua_istable(L, i)) { return (struct l_v2) { 0, 0 }; }

	lua_rawgeti(L, i, 1);
	lua_rawgeti(L, i, 2);
	r.x = luaL_checknumber(L, -2);
	r.y = luaL_checknumber(L, -1);

	lua_pop(L, 2);

	return r;
}

static struct mml_rect check_rect(lua_State* L, int i) {
	struct mml_rect r;
	if (!lua_istable(L, i)) { return (struct mml_rect) { 0, 0, 0, 0 }; }

	lua_rawgeti(L, i, 1);
	lua_rawgeti(L, i, 2);
	lua_rawgeti(L, i, 3);
	lua_rawgeti(L, i, 4);
	r.x = luaL_checknumber(L, -4);
	r.y = luaL_checknumber(L, -3);
	r.w = luaL_checknumber(L, -2);
	r.h = luaL_checknumber(L, -1);

	lua_pop(L, 4);

	return r;
}

static struct mml_pixel check_pixel(lua_State* L, int i) {
	struct mml_pixel r = { 255, 255, 255, 255 };
	if (!lua_istable(L, i)) { return r; }

	lua_rawgeti(L, i, 1);
	lua_rawgeti(L, i, 2);
	lua_rawgeti(L, i, 3);
	lua_rawgeti(L, i, 4);
	r.r = luaL_checknumber(L, -4);
	r.g = luaL_checknumber(L, -3);
	r.b = luaL_checknumber(L, -2);
	r.a = luaL_optnumber(L, -1, 255);

	lua_pop(L, 4);

	return r;
}

static void check_lua(lua_State* L, int r) {
	if (r != LUA_OK) {
		printf("%s\n", lua_tostring(L, -1));
	}
}

static int l_mml_bmp_load(lua_State* L) {
	const char* path = luaL_checkstring(L, 2);

	struct l_bitmap* self = lua_newuserdata(L, sizeof(struct l_bitmap));
	self->managed = 1;
	struct mml_bitmap** bmp = &self->self;
	luaL_setmetatable(L, "Bitmap");
	*bmp = mml_load_bitmap(path);
	return 1;
}

static int l_mml_bmp_new(lua_State* L) {
	struct l_v2 size = check_v2(L, 2);

	struct l_bitmap* self = lua_newuserdata(L, sizeof(struct l_bitmap));
	self->managed = 1;
	struct mml_bitmap** bmp = &self->self;
	luaL_setmetatable(L, "Bitmap");
	*bmp = mml_new_bitmap((int)size.x, (int)size.y);
	return 1;
}

static int l_mml_bmp_gc(lua_State* L) {
	struct l_bitmap* self = luaL_checkudata(L, 1, "Bitmap");
	struct mml_bitmap* bmp = self->self;
	if (bmp && self->managed) { mml_free_bitmap(bmp); }
	return 0;
}

static int l_mml_bmp_width(lua_State* L) {
	struct mml_bitmap* self = ((struct l_bitmap*)luaL_checkudata(L, 1, "Bitmap"))->self;
	lua_pushnumber(L, self->w);
	return 1;
}

static int l_mml_bmp_height(lua_State* L) {
	struct mml_bitmap* self = ((struct l_bitmap*)luaL_checkudata(L, 1, "Bitmap"))->self;
	lua_pushnumber(L, self->h);
	return 1;
}

static int l_mml_bmp_copy(lua_State* L) {
	struct mml_bitmap* self =  ((struct l_bitmap*)luaL_checkudata(L, 1, "Bitmap"))->self;
	struct mml_bitmap* other = ((struct l_bitmap*)luaL_checkudata(L, 2, "Bitmap"))->self;

	struct l_v2 pos = check_v2(L, 3);
	struct mml_rect r = check_rect(L, 4);
	struct mml_pixel c = check_pixel(L, 5);
	double scale = luaL_optnumber(L, 6, 1);

	mml_render_copy_ex(other, (int)pos.x, (int)pos.y, self, r, c, (int)scale);

	return 0;
}

static int l_mml_bmp_fill(lua_State* L) {
	struct mml_bitmap* self =  ((struct l_bitmap*)luaL_checkudata(L, 1, "Bitmap"))->self;

	struct mml_pixel c = check_pixel(L, 2);

	mml_render_fill(self, c);

	return 0;
}

static int l_mml_bmp_fill_rect(lua_State* L) {
	struct mml_bitmap* self =  ((struct l_bitmap*)luaL_checkudata(L, 1, "Bitmap"))->self;

	struct mml_rect r = check_rect(L, 2);
	struct mml_pixel c = check_pixel(L, 3);

	mml_fill_rect(self, r, c);

	return 0;
}

static int l_mml_bmp_text(lua_State* L) {
	struct mml_bitmap* self =  ((struct l_bitmap*)luaL_checkudata(L, 1, "Bitmap"))->self;
	struct mml_font** font = luaL_checkudata(L, 2, "Font");

	const char* text = luaL_checkstring(L, 3);

	struct l_v2 pos = check_v2(L, 4);
	struct mml_pixel c = check_pixel(L, 5);

	mml_render_text(self, *font, text, (int)pos.x, (int)pos.y, c);

	return 1;
}

static const luaL_Reg bitmap_lib[] = {
	{ "__gc",        l_mml_bmp_gc },
	{ "load",        l_mml_bmp_load },
	{ "new",         l_mml_bmp_new },
	{ "copy",        l_mml_bmp_copy },
	{ "width",       l_mml_bmp_width },
	{ "height",      l_mml_bmp_height },
	{ "fill",        l_mml_bmp_fill },
	{ "fill_rect",   l_mml_bmp_fill_rect },
	{ "render_text", l_mml_bmp_text },
	{ NULL, NULL }
};

static void register_bitmap(lua_State* L) {
	luaL_newmetatable(L, "Bitmap");
	luaL_setfuncs(L, bitmap_lib, 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	lua_pushvalue(L, -1);
	lua_setglobal(L, "Bitmap");

	lua_settop(L, 0);
}

static int l_mml_font_load(lua_State* L) {
	const char* path = luaL_checkstring(L, 2);
	double size = luaL_checknumber(L, 3);

	struct mml_font** self = lua_newuserdata(L, sizeof(struct mml_font*));
	*self = mml_load_font(path, size);
	luaL_setmetatable(L, "Font");
	return 1;
}

static int l_mml_font_gc(lua_State* L) {
	struct mml_font** self = luaL_checkudata(L, 1, "Font");
	if (*self) { mml_free_font(*self); }
	return 0;
}

static int l_mml_font_width(lua_State* L) {
	const char* text = luaL_checkstring(L, 2);

	struct mml_font** self = luaL_checkudata(L, 1, "Font");
	lua_pushnumber(L, mml_text_width(*self, text));
	return 1;
}

static int l_mml_font_height(lua_State* L) {
	struct mml_font** self = luaL_checkudata(L, 1, "Font");
	lua_pushnumber(L, mml_text_height(*self));
	return 1;
}

static const luaL_Reg font_lib[] = {
	{ "__gc",        l_mml_font_gc },
	{ "load",        l_mml_font_load },
	{ "width",       l_mml_font_width },
	{ "height",      l_mml_font_height },
	{ NULL, NULL }
};

static void register_font(lua_State* L) {
	luaL_newmetatable(L, "Font");
	luaL_setfuncs(L, font_lib, 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	lua_pushvalue(L, -1);
	lua_setglobal(L, "Font");

	lua_settop(L, 0);
}

static int l_mml_window_new(lua_State* L) {
	const char* name = luaL_checkstring(L, 1);
	double w = luaL_checknumber(L, 2);
	double h = luaL_checknumber(L, 3);
	double pix_size = luaL_checknumber(L, 4);
	int flags = 0;

	lua_pushvalue(L, 5);
	lua_pushnil(L);

	if (!lua_istable(L, -2)) {
		luaL_error(L, "argument #5 of Window.new should be table.");
		goto notable;
	}

	while (lua_next(L, -2)) {
		lua_pushvalue(L, -2);

		const char* flag = lua_tostring(L, -2);

		if      (strcmp(flag, "resizable")  == 0) { flags |= MML_WINDOW_RESIZABLE; }
		else if (strcmp(flag, "stretchy")   == 0) { flags |= MML_WINDOW_STRETCH; }
		else if (strcmp(flag, "fullscreen") == 0) { flags |= MML_WINDOW_FULLSCREEN; }

		lua_pop(L, 2);
	}

notable:
	lua_pop(L, 1);

	struct mml_window** self = lua_newuserdata(L, sizeof(struct mml_window*));
	luaL_setmetatable(L, "Window");
	*self = mml_new_window(name, (int)w, (int)h, (int)pix_size, flags);
	if (!*self) { luaL_error(L, "failed to open window."); }
	return 1;
}

static int l_mml_window_gc(lua_State* L) {
	struct mml_window** self = luaL_checkudata(L, 1, "Window");
	if (*self) { mml_free_window(*self); }
	return 0;
}

static int l_mml_window_next_event(lua_State* L) {
	struct mml_window** self = luaL_checkudata(L, 1, "Window");
	if (!*self) { return 0; }

	struct mml_event e;

	int r = mml_next_event(*self, &e);

	if (!lua_istable(L, 2)) {
		goto end;
	}

	const char* etn = "unknown";
	switch (e.type) {
		case MML_QUIT:
			etn = "quit";
			break;
		case MML_MOUSE_MOVE:
			etn = "mouse move";

			lua_newtable(L);

			lua_pushnumber(L, e.as.mouse_move_event.x);
			lua_setfield(L, -2, "x");

			lua_pushnumber(L, e.as.mouse_move_event.y);
			lua_setfield(L, -2, "y");

			lua_setfield(L, 2, "mouse_move");
			break;
		case MML_MOUSE_PRESS: {
			etn = "mouse press";

			const char* str = "unknown";

			switch (e.as.mouse_button_event.button) {
				case MML_MOUSE_LEFT:
					str = "left";
					break;
				case MML_MOUSE_RIGHT:
					str = "right";
					break;
				case MML_MOUSE_MIDDLE:
					str = "middle";
					break;
				default: break;
			}

			lua_pushstring(L, str);
			lua_setfield(L, 2, "button");
			break;

		}
		case MML_MOUSE_RELEASE: {
			etn = "mouse release";

			const char* str = "unknown";

			switch (e.as.mouse_button_event.button) {
				case MML_MOUSE_LEFT:
					str = "left";
					break;
				case MML_MOUSE_RIGHT:
					str = "right";
					break;
				case MML_MOUSE_MIDDLE:
					str = "middle";
					break;
				default: break;
			}

			lua_pushstring(L, str);
			lua_setfield(L, 2, "button");
			break;

		}
		case MML_KEY_PRESS: {
			etn = "key press";

			const char* str = "unknown";

			int k = e.as.keyboard_event.key;
			if (k >= 0 && k < (int)(sizeof(key_str) / sizeof(int))) {
				str = key_str[k];
			}

			lua_pushstring(L, str);
			lua_setfield(L, 2, "key");
			break;
		}
		case MML_KEY_RELEASE: {
			etn = "key release";

			const char* str = "unknown";

			int k = e.as.keyboard_event.key;
			if (k >= 0 && k < (int)(sizeof(key_str) / sizeof(int))) {
				str = key_str[k];
			}

			lua_pushstring(L, str);
			lua_setfield(L, 2, "key");
		}
		default: break;
	}

	lua_pushstring(L, etn);
	lua_setfield(L, 2, "type");
	lua_pop(L, 1);

end:
	lua_pushboolean(L, r);
	return 1;
}

static int l_mml_window_query(lua_State* L) {
	struct mml_window** self = luaL_checkudata(L, 1, "Window");
	if (!*self) { return 0; }

	int w, h, pixel_size, flags;
	mml_query_window(*self, &flags, &w, &h, &pixel_size);

	lua_newtable(L);

	lua_pushnumber(L, w);
	lua_setfield(L, -2, "w");

	lua_pushnumber(L, h);
	lua_setfield(L, -2, "h");

	lua_pushnumber(L, pixel_size);
	lua_setfield(L, -2, "pixel_size");

	lua_newtable(L);

	int i = 1;

	if (flags & MML_WINDOW_RESIZABLE) {
		lua_pushstring(L, "resizable");
		lua_rawseti(L, -2, i++);
	}

	if (flags & MML_WINDOW_STRETCH) {
		lua_pushstring(L, "stretchy");
		lua_rawseti(L, -2, i++);
	}

	if (flags & MML_WINDOW_FULLSCREEN) {
		lua_pushstring(L, "fullscreen");
		lua_rawseti(L, -2, i++);
	}

	lua_setfield(L, -2, "flags");

	return 1;
}

static int l_mml_window_update(lua_State* L) {
	struct mml_window** self = luaL_checkudata(L, 1, "Window");
	if (!*self) { return 0; }

	mml_update_window(*self);

	return 0;
}

static int l_mml_window_backbuffer(lua_State* L) {
	struct mml_window** self = luaL_checkudata(L, 1, "Window");
	if (!*self) { return 0; }

	struct l_bitmap* sbmp = lua_newuserdata(L, sizeof(struct l_bitmap));
	luaL_setmetatable(L, "Bitmap");
	sbmp->managed = 0;
	sbmp->self = mml_get_backbuffer(*self);

	return 1;
}

static const luaL_Reg window_lib[] = {
	{ "__gc",        l_mml_window_gc },
	{ "new",         l_mml_window_new },
	{ "next_event",  l_mml_window_next_event },
	{ "query",       l_mml_window_query },
	{ "update",      l_mml_window_update },
	{ "backbuffer",  l_mml_window_backbuffer },
	{ NULL, NULL }
};

static void register_window(lua_State* L) {
	luaL_newmetatable(L, "Window");
	luaL_setfuncs(L, window_lib, 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	lua_pushvalue(L, -1);
	lua_setglobal(L, "Window");

	lua_settop(L, 0);
}

static int l_mml_init(lua_State* L) {
	mml_init();
	return 0;
}

static int l_mml_deinit(lua_State* L) {
	mml_deinit();
	return 0;
}

static int l_mml_get_time(lua_State* L) {
	lua_pushnumber(L, mml_get_time());
	return 1;
}

static int l_mml_get_frequency(lua_State* L) {
	lua_pushnumber(L, mml_get_frequency());
	return 1;
}

static const luaL_Reg lib[] = {
	{ "init",         l_mml_init },
	{ "deinit",       l_mml_deinit },
	{ "get_time",     l_mml_get_time },
	{ "get_frequency",l_mml_get_frequency },
	{ NULL, NULL }
};

static int open_mml(lua_State* L) {
	luaL_newlib(L, lib);
	return 1;
}

int main(int argc, char** argv) {
	srand((unsigned int)time(NULL));

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaL_requiref(L, "mml", open_mml, 1);
	register_window(L);
	register_bitmap(L);
	register_font(L);

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			int r = luaL_dofile(L, argv[i]);
			check_lua(L, r);
		}
	} else {
		printf("No input files.\n");
		lua_close(L);
		return 1;
	}

	lua_close(L);

	return 0;
}
