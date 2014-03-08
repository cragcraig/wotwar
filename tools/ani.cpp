#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <allegro.h>

// functions
void die_callback(void);

// Timer
volatile long speed_counter = 0;
void increment_speed_counter()
{
	speed_counter++;
}
END_OF_FUNCTION(increment_speed_counter);

bool lets_die = false;

// Main
int main(int argc, char * argv[])
{	
	allegro_init(); // init text-mode allegro
	set_color_depth(32); // so any bitmaps we create are 32 bit
	
	// check that we got a filename
	if (argc != 2 && argc != 3) {
		allegro_message("usage error! Must be run from command line with the following format.\n\nusage: ani filename.bmp [frame_width]");
		return -1;
	}
	
	// variables
	int frames;
	int width;
	int mult;
	bool pressed = false;
	int c = 0, f = 0, s = 4;
	
	// setup
	install_keyboard();
	install_timer();
	LOCK_VARIABLE(speed_counter);
	LOCK_FUNCTION(increment_speed_counter);
	install_int_ex(increment_speed_counter, BPS_TO_TIMER(40));
	set_close_button_callback(die_callback); // function to end program
	set_window_title("ani preview"); // set window's title
	if (set_gfx_mode(GFX_SAFE, 0, 0, 0, 0) < 0) {
		printf("error: %s\n", allegro_error);
		return -1;
	}
    
    BITMAP* ani = load_bitmap(argv[1], NULL);
    if (!ani) {
		allegro_message("Error: image not found");
		return -1;
	}
	
	if (argc == 3) {
		sscanf(argv[2], "%u", &mult);
	} else mult = 1;
	
	width = ani->h * mult;
	frames = ani->w / width;
	
	if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, width, ani->h, 0, 0) < 0) {
		printf("unable to adjust window size: %s\n", allegro_error);
		if (set_gfx_mode(GFX_SAFE, 0, 0, 0, 0) < 0) {
			printf("error: %s\n", allegro_error);
			return -1;
		}
	}

	// show animation
	speed_counter = 0;
	
	while (!key[KEY_ESC] && !lets_die) {
		if (speed_counter > 0) {
			speed_counter--;
			if (!c--) {
				c = s;
				blit(ani, screen, width*(f%frames), 0, 0, 0, width, ani->h);
				textprintf_ex(screen, font, 1, 1, 0, -1, "rate: %d", s);
				f++;
			}
			if (key[KEY_UP] && !pressed) s++, pressed = true;
			if (key[KEY_DOWN] && !pressed) s--, pressed = true;
			if (!key[KEY_UP] && !key[KEY_DOWN]) pressed = false;
			if (s < 1) s = 1;
		}
	}
	
	// release data
	destroy_bitmap(ani);

	return 0;
}
END_OF_MAIN();

void die_callback(void)
{
	lets_die = true;
}END_OF_FUNCTION(die_callback)
