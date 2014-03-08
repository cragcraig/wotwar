#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "wotwar.h"

bool skipline(char* line) {
	if (strrchr(line, '#') || strspn(line, " \t") == strlen(line)-1) return true;
	else return false;
}

//---------------------------------------------------------------------------------------------
//		GLOBAL CONFIG FUNCTIONS
//---------------------------------------------------------------------------------------------

// Global config settings and defaults
void init_config(GLOBAL_CONFIG& c)
{
	c.GAME_TITLE[0] = '\0';
	c.GAME_CREDITS[0] = '\0';
	c.SCREEN_WIDTH = 800;
	c.SCREEN_HEIGHT = 600;
	c.PAGE_FLIPPING = true;
	c.VIDEOMEM_BACKBUFFER = true;
	c.SCREEN_WINDOWED = false;
	c.CONFIG_FADE_TILES = true;
	c.NETWORK_PORT = 50000;
	c.GAME_TIMEBEGAN = time(NULL);
}

// Global config load from file
void load_config(const char* filename, GLOBAL_CONFIG& c)
{
	// attempt to open config file
	FILE* fd = fopen(filename,"r");
	if (!fd) {
		printf("No config file found, just using defaults.\n");
		return;
	}
	int tmp;
	// read title and credits
	fgets(c.GAME_TITLE, sizeof(c.GAME_TITLE), fd);
	strip_nonprintables(c.GAME_TITLE);
	fgets(c.GAME_CREDITS, sizeof(c.GAME_CREDITS), fd);
	strip_nonprintables(c.GAME_CREDITS);
	// load settings (need the space in front for it to ignore all whitespace)
	if (fscanf(fd, " xres=%u", &tmp)) c.SCREEN_WIDTH = tmp;
	if (fscanf(fd, " yres=%u", &tmp)) c.SCREEN_HEIGHT = tmp;
	if (fscanf(fd, " pageflipping=%u", &tmp)) c.PAGE_FLIPPING = tmp;
	if (fscanf(fd, " videomem_backbuffer=%u", &tmp))c.VIDEOMEM_BACKBUFFER = tmp;
	if (fscanf(fd, " windowed=%u", &tmp)) c.SCREEN_WINDOWED = tmp;
	if (fscanf(fd, " fade_tiles=%u", &tmp)) c.CONFIG_FADE_TILES = tmp;
	if (fscanf(fd, " net_port=%u", &tmp)) c.NETWORK_PORT = tmp;
	fclose(fd);
	puts(c.GAME_TITLE);
	printf("Loading general configuration file: '%s'\n", filename);
	printf(" xres=%u\n yres=%u\n windowed=%u\n\n", c.SCREEN_WIDTH, c.SCREEN_HEIGHT, c.SCREEN_WINDOWED);
}
