#include <stdio.h>

#include "allegro5/allegro5.h"
#include "a5_font.h"


static bool key_down(void)
{
	ALLEGRO_KBDSTATE kbdstate;

	al_get_keyboard_state(&kbdstate);

	int i;

	for (i = 0; i < ALLEGRO_KEY_MAX; i++) {
		if (al_key_down(&kbdstate, i)) {
			return true;
		}
	}

	return false;
}

static void print(A5FONT_FONT *myfont, char *message, int x, int y)
{
	ALLEGRO_COLOR color;

	al_set_blender(ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA,
      al_map_rgb(0, 0, 0));
	a5font_textout(myfont, message, x+2, y+2);

	al_set_blender(ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA,
      al_map_rgb(255, 255, 255));
	a5font_textout(myfont, message, x, y);
}

static void test(ALLEGRO_BITMAP *bitmap, A5FONT_FONT *font, char *message)
{
	long frames = 0;
	long start_time = al_current_time();
	int fps = 0;

	for (;;) {
		if (key_down()) {
			while (key_down())
				;
			break;
		}

		al_set_blender(ALLEGRO_ONE, ALLEGRO_ZERO,
            al_map_rgb(255, 255, 255));

		al_draw_scaled_bitmap(bitmap, 0, 0,
			al_get_bitmap_width(bitmap),
			al_get_bitmap_height(bitmap),
			0, 0,
			al_get_display_width(),
			al_get_display_height(),
			0);

		print(font, message, 0, 0);
		char second_line[100];
		sprintf(second_line, "%d FPS", fps);
		print(font, second_line, 0, a5font_text_height(font)+5);

		al_flip_display();

		frames++;
		long now = al_current_time();
		long mseconds = now - start_time;
		fps = mseconds > 10 ? 1000 * frames / mseconds : 0;
	}
}

int main(void)
{
   al_init();

   al_install_keyboard();

   ALLEGRO_DISPLAY *display = al_create_display(640, 400);

   A5FONT_FONT *accelfont = a5font_load_font("font.tga", 0);
   ALLEGRO_BITMAP *accelbmp = al_load_bitmap("mysha.pcx");

   al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);

   A5FONT_FONT *memfont = a5font_load_font("font.tga", 0);
   ALLEGRO_BITMAP *membmp = al_load_bitmap("mysha.pcx");

   test(membmp, memfont, "Memory bitmap");

   test(accelbmp, accelfont, "Accelerated bitmap");

   return 0;
}
END_OF_MAIN()

