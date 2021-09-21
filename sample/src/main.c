#include <stdio.h>

#include <mml.h>

int main() {
	struct mml_window* window;
	int running;
	struct mml_event event;
	struct mml_bitmap* backbuffer, *logo;
	struct mml_font* font;

	char buf[256];

	unsigned long long now, last;
	double frame_time, timer;

	frame_time = timer = 0.0;
	now = last = 0;

	mml_init();

	window = mml_new_window("MML Sample", 640, 480, 1, MML_WINDOW_RESIZABLE | MML_WINDOW_STRETCH);

	logo = mml_load_bitmap("mml.bmp");

	font = mml_load_font("src/jetbrainsmono.ttf", 14.0f);

	running = 1;
	while (running) {
		while (mml_next_event(window, &event)) {
			switch (event.type) {
				case MML_QUIT:
					running = 0;
					break;
				case MML_MOUSE_PRESS:
					printf("mouse button press: %d\n", event.as.mouse_button_event.button);
					break;
				case MML_MOUSE_RELEASE:
					printf("mouse button release: %d\n", event.as.mouse_button_event.button);
					break;
				case MML_KEY_PRESS:
					printf("press: %d\n", event.as.keyboard_event.key);
					break;
				case MML_KEY_RELEASE:
					printf("release: %d\n", event.as.keyboard_event.key);
					break;
				case MML_MOUSE_MOVE:
					printf("mouse move: %d, %d\n",
							event.as.mouse_move_event.x,
							event.as.mouse_move_event.y);
				default:
					break;
			}
		}

		now = mml_get_time();
		frame_time = ((double)now - (double)last) / (double)mml_get_frequency();
		last = now;

		timer += frame_time;
		if (timer > 1.0) {
			sprintf(buf, "frame time: %g; fps: %g", frame_time, 1.0 / frame_time);
			timer = 0.0;
		}

		backbuffer = mml_get_backbuffer(window);
		mml_render_fill(backbuffer, mml_color(0, 0, 0, 255));

		mml_render_copy_ex(backbuffer, 0, 0, logo, mml_rect(0, 0, logo->w, logo->h), (struct mml_pixel) { 255, 255, 255, 255 }, 1);
		mml_fill_rect(backbuffer, mml_rect(logo->w, logo->h, 100, 100), mml_color(0, 255, 255, 255));
		mml_render_text(backbuffer, font, buf, 0, 0, mml_color(255, 255, 255, 255));

		mml_update_window(window);
	}

	mml_free_font(font);

	mml_free_bitmap(logo);

	mml_free_window(window);

	mml_deinit();

	return 0;
}
