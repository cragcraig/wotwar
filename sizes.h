#ifndef SIZES_H
#define SIZES_H

#include <string>

#define ENABLE_DEBUG // if debug mode is on, disable tricksy stuff so gdb doesn't get interrupted with signals

// posix compliant uses pause() and signals instead of the normal busy wait
#if ((defined(unix) || defined(__unix__) || defined(__unix)) && !defined(ENABLE_DEBUG))
# include <unistd.h>
# if (defined(_POSIX_VERSION) && _POSIX_VERSION >= 200112L)
#  define POSIX_COMPLIANT 1
# endif
#endif

// general settings
#define LOG_STDOUT 1
#define LOW_QUALITY_TILE_FADING 1 // lower quality (but faster) tile fading
#define MAX_FPS 50
#define MAX_TILE_NAME_LENGTH 64
#define GOAL_FPS 40
#define MAX_FRAMELAG 4
#define MAX_FPSDEBT 50
#define MAX_ONSCREEN_MESSAGES 7
#define MAX_ONSCREEN_MSGTIME 30
#define PLAYER_MESSAGE_MAXLENGTH 256
#define RESOURCE_UPDATE_FREQ 15
#define GROUP_MAXUNITS 30 // don't set too high, these should all fit in a TCP packet for performance
#define NETWORK_ALLOWABLE_GROUPERROR 75 // if this much error (px) the group positions will be forcefully synced

// network settings
#define FILE_PACKET_SIZE 1024
#define HEADER_PACKET_SIZE 128
#define MESSAGE_BUFFER_LEN 256

// TEXTURE_SIZE should be an integer multiple of TILES_PER_TEXTURE
#define TEXTURE_SIZE 512
#define TILES_PER_TEXTURE 8
#define TILE_SIZE (TEXTURE_SIZE/TILES_PER_TEXTURE)

// STATIONARY_SIZE should be an interger multiple of TILE_SIZE
#define STATIONARY_SIZE 192
#define TILES_PER_STATIONARY (STATIONARY_SIZE/TILE_SIZE)

// Pather Stuff
#define PATCH_SIZE 32
#define SMALL_AREA_SIZE STATIONARY_SIZE // don't change this!
#define TILES_PER_SMALLAREA (SMALL_AREA_SIZE/TILE_SIZE)
#define BUFFER_AREA_STATIONARIES 0
#define EXTRA_AREA_STATIONARIES 0

// Pathfinding
#define PATHFIND_MAX_SEARCH_NODES 2000
#define NEAR_STATIONARY_COST 1

// Game
#define SCROLL_WIDTH 15.0
#define DRAG_DIST_TO_ARROW 100
#define MINIMAP_TIME_TO_MOVE 7
#define DIST_TO_CLAIM_BUILDSITE (10 * TILE_SIZE)

// World stuff
#define WORLD_RENDER_BORDER_SIZE TILE_SIZE // should be the same as TILE_SIZE for efficiency purposes

// MISC STUFF
#define NUM_OF_STATIONARIES_TO_SWITCH_DRAWING_MODE 20 // number of stationaries on screen as cuttoff to switch to alternate batch drawing mode
#define ARROW_OFFSET 50 // offset from group center to draw direction arrow
#define POPUP_SIZE 75
#define MAX_BUILDS 50 // max number of unit types to be built from a single building
#define MAX_MAP_SIZE 200
#define MIN_MAP_SIZE 50
#define AGENT_SELECT_DIST 50

// handing pixel drawing macros
#define PUTPIXELS_SQUARE(buf, x, y, color) {putpixel(buf, x, y, color); putpixel(buf, x+1, y, color); putpixel(buf, x, y+1, color); putpixel(buf, x+1, y+1, color);}
#define PUTPIXELS_X(buf, x, y, color) {putpixel(buf, x, y, color); putpixel(buf, x+1, y+1, color); putpixel(buf, x-1, y+1, color); putpixel(buf, x+1, y-1, color); putpixel(buf, x-1, y-1, color);}

// The actual variable is in main.cpp
struct GLOBAL_CONFIG
{
	char GAME_TITLE[256];
	char GAME_CREDITS[1024];
	int SCREEN_WIDTH;
	int SCREEN_HEIGHT;
	int PAGE_FLIPPING;
	int VIDEOMEM_BACKBUFFER;
	int SCREEN_WINDOWED;
	int CONFIG_FADE_TILES;
	int NETWORK_PORT;
	time_t GAME_TIMEBEGAN;
};

// string compare object
struct cfg_cmp { bool operator()(const std::string& a, const std::string& b) const { return (a.compare(b) < 0); } };

#endif
