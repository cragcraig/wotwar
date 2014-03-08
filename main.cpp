// Craig Harrison 2007-2008
// Wheel of Time War (WoT War)

#include "wotwar_classes.h"
#include "wotwar.h"
#include <cctype>

#if POSIX_COMPLIANT
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
  void dummy_wake_sleep_handle(int r) {}
  pid_t pid;
#endif

// Functions / Global Variables
GLOBAL_CONFIG global_config;
bool DIE_NOW = false;
static GUI * cur_gui = NULL;
int fps_debt = 0;

// Functions
void init_libraries(GLOBAL_CONFIG& c);
void open_log(bool stdout_to_file);

// Timer
volatile long speed_counter = 0;
void increment_speed_counter()
{
	speed_counter++;
#if POSIX_COMPLIANT
	kill(pid, SIGUSR1); // wakey-wakey
#endif
}
END_OF_FUNCTION(increment_speed_counter);

// Main
int main(void)
{
#if POSIX_COMPLIANT
	pid = getpid();
	signal(SIGUSR1, dummy_wake_sleep_handle); // need to catch the signal
#endif

	// Game state
	init_config(global_config);

	// Redirect stdout to log file
	open_log(LOG_STDOUT);

	// load config and init allegro
	load_config("data/settings.conf", global_config);
	init_libraries(global_config);

	// Setup screen and buffer
	SCREEN *video_screen = new SCREEN(global_config.SCREEN_WIDTH, global_config.SCREEN_HEIGHT, global_config.SCREEN_WINDOWED, global_config.PAGE_FLIPPING, global_config.VIDEOMEM_BACKBUFFER);

	// setup and load gui
    GUI *gui = new GUI(global_config.GAME_TITLE, *video_screen);
    cur_gui = gui;

	// Screenshot key state, this way it only takes one
	bool s_key = false;

	// FPS key
	unsigned int lastTime = time(0);
	int fpsCounter = 0;
	int FPS = 0;
	int draw_screen = true;

	// Reset FPS
	speed_counter = 0;

	// Main game loop
	while (!DIE_NOW) {
		if (speed_counter > 0) {
			speed_counter--;
			draw_screen = speed_counter < MAX_FRAMELAG ? true : false;

			// COMPUTE FPS
			if (lastTime == time(0)) fpsCounter++;
			else {
				lastTime = time(0);
				FPS = fpsCounter;
				fpsCounter = 1;
				fps_debt = speed_counter;
			}

			// DO THE FRAME
			gui->update(draw_screen);

			// SCREENSHOT
			if (key[KEY_S] && (key_shifts & KB_CTRL_FLAG) && !s_key) {
				video_screen->take_screenshot("screenshot.bmp");
				s_key=true;
				gui->message("Screenshot", "Saved");
			}
			if (!key[KEY_S]) s_key = false;

			// DRAW FPS
            if (key[KEY_F]) {
                textprintf_ex(video_screen->get_page(), font, 5, 5, makecol(255,255,255), makecol(0,0,0), "fps: %d", FPS);
                textprintf_ex(video_screen->get_page(), font, 5, 5 + text_height(font), makecol(255,255,255), makecol(0,0,0), "debt: %d", fps_debt);
            }

			// UPDATE SCREEN
			if (draw_screen) video_screen->flip_page();
		}
#if POSIX_COMPLIANT
		else pause();
#endif
	}

	gui->end_game();

	// destructors must happen before allegro exits
	delete gui;
	delete video_screen;

	allegro_exit();

	return 0;
}
END_OF_MAIN();

// Library initialization
void init_libraries(GLOBAL_CONFIG& c)
{
	// install allegro
	set_uformat(U_ASCII);
	allegro_init();
	install_keyboard();
	install_timer();
	install_mouse();
	install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT,0);

	// get default screen resolution
	if (c.SCREEN_WIDTH <= 300 || c.SCREEN_HEIGHT <= 200) get_desktop_resolution(&c.SCREEN_WIDTH, &c.SCREEN_HEIGHT);

	// setup allegro stuff
	LOCK_VARIABLE(speed_counter);
	LOCK_FUNCTION(increment_speed_counter);
	install_int_ex(increment_speed_counter, BPS_TO_TIMER(GOAL_FPS));
	set_close_button_callback(NULL); // install callback function to nicely end program on GUI close-button click
	set_window_title(c.GAME_TITLE); // set window's title

	// misc stuff
	srand(time(0));
}

// log all output
void open_log(bool stdout_to_file)
{
	if (stdout_to_file) {
		printf("WoT War RTS Engine :: http://wotwar.googlecode.com\n> output being redirected to \"log.txt\"\n");
		freopen("log.txt", "w", stdout); // redirect stdout
		setvbuf(stdout, NULL, _IOLBF, 1024*sizeof(char)); // set stdout to a line buffer so we don't lose anything
	}
	printf("WoT War RTS Engine :: http://wotwar.googlecode.com\n\n");
	printf("time:\n%s\n", ctime(&global_config.GAME_TIMEBEGAN));
}

// End program nicely
void exit_game(void)
{
	DIE_NOW = true;
	puts("end_game() called.");
}

// Panic
void panic(const char* s)
{
	printf("\npanic: %s\nfatal error, unable to continue.\n", s);
	fprintf(stderr, "fatal runtime error: %s\nCheck log.txt for more information.\n", s);
	if (cur_gui) cur_gui->message("The Unexpected Occured", s);
	exit(-1);
}

// Message
void global_popup(const char* s)
{
	printf("global_popup: %s\n", s);
	if (cur_gui) cur_gui->message("", s);
}

// clear speed_counter
void clear_speedcounter(void)
{
    speed_counter = 0;
    fps_debt = 0;
}

// read fps debt
int get_fpsdebt(void)
{
    return fps_debt;
}

// read speed_counter
int get_speedcounter(void)
{
    return speed_counter;
}

void strip_nonprintables(char * str)
{
    while (*str != '\0') {
        if (!isprint(*str)) {
            *str = '\0';
            return;
        }
        str++;
    }
}
