#include <stdlib.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
#include "stb_truetype.h"



#include "mml.h"

#pragma pack(push, 1)
struct bmp_header {
	uint16_t ftype;
	uint32_t fsize;
	uint16_t res1, res2;
	uint32_t bmp_offset;
	uint32_t size;
	int32_t w, h;
	uint16_t planes;
	uint16_t bits_per_pixel;
};
#pragma pack(pop)

#define MML_MAX_GLYPHSET 256

static inline struct mml_pixel blend_pixel(struct mml_pixel dst, struct mml_pixel src, struct mml_pixel color) {
	int ia;

	src.a = (src.a * color.a) >> 8;
	ia = 0xff - src.a;
	dst.r = ((src.r * color.r * src.a) >> 16) + ((dst.r * ia) >> 8);
	dst.g = ((src.g * color.g * src.a) >> 16) + ((dst.g * ia) >> 8);
	dst.b = ((src.b * color.b * src.a) >> 16) + ((dst.b * ia) >> 8);
	return dst;
}

static const char* utf8_to_codepoint(const char* p, unsigned int* dst) {
	unsigned int res, n;
	switch (*p & 0xf0) {
		case 0xf0 : res = *p & 0x07; n = 3; break;
		case 0xe0 : res = *p & 0x0f; n = 2; break;
		case 0xd0 :
		case 0xc0 : res = *p & 0x1f; n = 1; break;
		default   : res = *p;        n = 0; break;
	}
	while (n--) {
		res = (res << 6) | (*(++p) & 0x3f);
	}
	*dst = res;
	return p + 1;
}

struct mml_bitmap* mml_new_bitmap(int w, int h) {
	struct mml_bitmap* bitmap;

	bitmap = malloc(sizeof(struct mml_bitmap) * w * h * sizeof(struct mml_pixel));

	bitmap->pixels = (struct mml_pixel*)(bitmap + 1);

	bitmap->w = w;
	bitmap->h = h;

	return bitmap;
}

void mml_free_bitmap(struct mml_bitmap* bitmap) {
	free(bitmap);
}

struct mml_bitmap* mml_load_bitmap(const char* filename) {
	int w, h, cc;
	struct mml_pixel* pixels;
	struct mml_bitmap* bitmap;

	pixels = (struct mml_pixel*)stbi_load(filename, &w, &h, &cc, 4);
	if (!pixels) {
		printf("Failed to load %s: %s\n", filename, stbi_failure_reason());
		return NULL;
	}

	bitmap = mml_new_bitmap(w, h);
	memcpy(bitmap->pixels, pixels, w * h * sizeof(struct mml_pixel));
	stbi_image_free(pixels);

	return bitmap;
}

struct fill_data {
	struct mml_pixel* ptr;
	int len;
	struct mml_pixel color;
};

static void fill_worker(void* udata) {
	struct fill_data* data;
	int i;

	data = (struct fill_data*)udata;

	for (i = 0; i < data->len; i++) {
		data->ptr[i] = data->color;
	}
}

void mml_render_fill(struct mml_bitmap* dst, struct mml_pixel color) {
	int i;

	for (i = 0; i < dst->w * dst->h; i++) {
		dst->pixels[i] = color;
	}
}

void mml_render_copy(struct mml_bitmap* dst, int x, int y, struct mml_bitmap* src, struct mml_rect src_rect) {
	mml_render_copy_ex(dst, x, y, src, src_rect, (struct mml_pixel) { 255, 255, 255, 255 }, 1);
}

void mml_render_copy_ex(struct mml_bitmap* dst, int x, int y, struct mml_bitmap* src, struct mml_rect src_rect,
		struct mml_pixel color, int scale) {

	int n, j, i, is, js, xx, yy;
	struct mml_pixel *d;

	struct mml_rect clip = {
		x >= 0 ? x : 0,
		y >= 0 ? y : 0,
		((dst->w / scale) - (x / scale)),
		((dst->h / scale) - (y / scale)),
	};

	if (scale <= 0) { return; }

	if (clip.x > dst->w) { return; }
	if (clip.y > dst->h) { return; }
	if ((n = clip.x - x) > 0) { src_rect.w -= n; src_rect.x += n; x += n; }
	if ((n = clip.y - y) > 0) { src_rect.h -= n; src_rect.y += n; y += n; }
	if ((n = x + src_rect.w - (clip.x + clip.w)) > 0) { src_rect.w -= n; }
	if ((n = y + src_rect.h - (clip.y + clip.h)) > 0) { src_rect.h -= n; }

	if (src_rect.w <= 0 || src_rect.h <= 0) {
		return;
	}

	if (scale > 1) {
		yy = src_rect.y;
		for (j = 0; j < src_rect.h; j++, yy++) {
			xx = src_rect.x;
			for (i = 0; i < src_rect.w; i++, xx++) {
				for (js = 0; js < scale; js++) {
					for (is = 0; is < scale; is++) {
						d = dst->pixels + (((x + i * scale) + is) + ((y + j * scale) + js) * dst->w);
						*d = blend_pixel(*d, src->pixels[xx + yy * src->w], color);
					}
				}
			}
		}
	} else {
		yy = src_rect.y;
		for (j = 0; j < src_rect.h; j++, yy++) {
			xx = src_rect.x;
			for (i = 0; i < src_rect.w; i++, xx++) {
				d = dst->pixels + ((x + i) + (y + j) * dst->w);
				*d = blend_pixel(*d, src->pixels[xx + yy * src->w], color);
			}
		}
	}
}

void mml_fill_rect(struct mml_bitmap* dst, struct mml_rect rect, struct mml_pixel color) {
	int x1, y1, x2, y2, dr, j, i;
	struct mml_pixel *d;

	struct mml_rect clip = {
		rect.x >= 0 ? rect.x : 0,
		rect.y >= 0 ? rect.y : 0,
		dst->w - rect.x,
		dst->h - rect.y,
	};

	x1 = rect.x < clip.x ? clip.x : rect.x;
	y1 = rect.y < clip.y ? clip.y : rect.y;
	x2 = rect.x + rect.w;
	y2 = rect.y + rect.h;
	x2 = x2 > (clip.x + clip.w) ? (clip.x + clip.w) : x2;
	y2 = y2 > (clip.y + clip.h) ? (clip.y + clip.h) : y2;

	d = dst->pixels;
	d += x1 + y1 * dst->w;
	dr = dst->w - (x2 - x1);

	if (color.a != 0xff) {
		for (j = y1; j < y2; j++) {
			for (i = x1; i < x2; i++) {
				*d = blend_pixel(*d, color, (struct mml_pixel) { 255, 255, 255, 255 });
				d++;
			}
			d += dr;
		}
	} else {
		for (j = y1; j < y2; j++) {
			for (i = x1; i < x2; i++) {
				*d = color;
				d++;
			}
			d += dr;
		}
	}
}

struct glyph_set {
	struct mml_bitmap* atlas;
	stbtt_bakedchar glyphs[256];
};

struct mml_font {
	void* data;
	stbtt_fontinfo info;
	struct glyph_set* sets[MML_MAX_GLYPHSET];
	float size;
	int height;
};

static struct glyph_set* load_glyph_set(struct mml_font* font, int idx) {
	int width, height, r, ascent, descent, linegap, scaled_ascent, i;
	unsigned char n;
	float scale, s;
	struct glyph_set* set;

	set = calloc(1, sizeof(struct glyph_set));

	width = 128;
	height = 128;

retry:
	set->atlas = mml_new_bitmap(width, height);

	s = stbtt_ScaleForMappingEmToPixels(&font->info, 1) /
		stbtt_ScaleForPixelHeight(&font->info, 1);
	r = stbtt_BakeFontBitmap(font->data, 0, font->size * s,
			(void*)set->atlas->pixels, width, height, idx * 256, 256, set->glyphs);

	if (r <= 0) {
		width *= 2;
		height *= 2;
		mml_free_bitmap(set->atlas);
		goto retry;
	}

	stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &linegap);
	scale = stbtt_ScaleForMappingEmToPixels(&font->info, font->size);
	scaled_ascent = (int)(ascent * scale + 0.5);
	for (i = 0; i < 256; i++) {
		set->glyphs[i].yoff += scaled_ascent;
		set->glyphs[i].xadvance = (float)floor(set->glyphs[i].xadvance);
	}

	for (i = width * height - 1; i >= 0; i--) {
		n = *((unsigned char*)set->atlas->pixels + i);
		set->atlas->pixels[i] = (struct mml_pixel) {255, 255, 255, n};
	}

	return set;
}

static struct glyph_set* get_glyph_set(struct mml_font* font, int code_point) {
	int idx;
	
	idx = (code_point >> 8) % MML_MAX_GLYPHSET;
	if (!font->sets[idx]) {
		font->sets[idx] = load_glyph_set(font, idx);
	}
	return font->sets[idx];
}

struct mml_font* mml_load_font(const char* filename, float size) {
	void* data;
	unsigned int filesize;
	
	if (mml_read_file(filename, &data, &filesize, 0) != MML_OK) {
		return NULL;
	}

	return mml_load_font_from_memory(data, filesize, size);
}

struct mml_font* mml_load_font_from_memory(void* data, int filesize, float size) {
	struct mml_font* font;
	int r, ascent, descent, linegap;
	float scale;

	font = calloc(1, sizeof(struct mml_font));
	font->data = data;

	font->size = size;

	r = stbtt_InitFont(&font->info, font->data, 0);
	if (!r) {
		goto fail;
	}

	stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &linegap);
	scale = stbtt_ScaleForMappingEmToPixels(&font->info, size);
	font->height = (int)((ascent - descent + linegap) * scale + 0.5);

	stbtt_bakedchar* g = get_glyph_set(font, '\n')->glyphs;
	g['\t'].x1 = g['\t'].x0;
	g['\n'].x1 = g['\n'].x0;

	mml_set_font_tab_size(font, 8);

	return font;

fail:
	if (font->data) { free(font->data); }
	if (font) { free(font); }
	return NULL;
}

void mml_free_font(struct mml_font* font) {
	int i;
	struct glyph_set* set;

	for (i = 0; i < MML_MAX_GLYPHSET; i++) {
		set = font->sets[i];
		if (set) {
			if (set->atlas) {
				mml_free_bitmap(set->atlas);
			}
			free(set);
		}
	}

	free(font->data);
	free(font);
}

void mml_set_font_tab_size(struct mml_font* font, int n) {
	struct glyph_set* set;

	set = get_glyph_set(font, '\t');
	set->glyphs['\t'].xadvance = n * set->glyphs[' '].xadvance;
}

int mml_get_font_tab_size(struct mml_font* font) {
	struct glyph_set* set;

	set = get_glyph_set(font, '\t');
	return (int)(set->glyphs['\t'].xadvance / set->glyphs[' '].xadvance);
}

int mml_text_width(struct mml_font* font, const char* text) {
	int x;
	unsigned int codepoint;
	const char* p;
	struct glyph_set* set;
	stbtt_bakedchar* g;
	
	x = 0;
	p = text;
	while (*p) {
		p = utf8_to_codepoint(p, &codepoint);
		set = get_glyph_set(font, codepoint);
		g = &set->glyphs[codepoint & 0xff];
		x += (int)g->xadvance;
	}
	return x;
}

int mml_text_height(struct mml_font* font) {
	return font->height;
}

int mml_render_text(struct mml_bitmap* dst, struct mml_font* font,
		const char* text, int x, int y, struct mml_pixel color) {
	struct mml_rect r;
	const char* p;
	unsigned int codepoint;
	struct glyph_set* set;
	stbtt_bakedchar* g;

	p = text;
	while (*p) {
		p = utf8_to_codepoint(p, &codepoint);

		set = get_glyph_set(font, codepoint);
		g = &set->glyphs[codepoint & 0xff];

		r.x = g->x0;
		r.y = g->y0;
		r.w = g->x1 - g->x0;
		r.h = g->y1 - g->y0;
		mml_render_copy_ex(dst, (int)(x + g->xoff), (int)(y + g->yoff), set->atlas, r, color, 1);
		x += (int)g->xadvance;
	}

	return x;
}
