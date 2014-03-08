#include "wotwar.h"

SCREEN::SCREEN(int width, int height, bool windowed, bool db, bool videobuf_tiles) : w(width), h(height), doublebuffer(db)
{
    if (!doublebuffer) { // page flipping
        // set graphics mode
        set_color_depth(32);
        if (set_gfx_mode((windowed ? GFX_AUTODETECT_WINDOWED : GFX_AUTODETECT_FULLSCREEN), width, height, width, height * 2) != 0) { // get video memory now (smart drivers)
            printf("warning: set_gfx_mode(): %s\n", allegro_error);
            if (set_gfx_mode((windowed ? GFX_AUTODETECT_WINDOWED : GFX_AUTODETECT_FULLSCREEN), width, height, 0, 0) != 0) { // just open screen now and hope we can get the video memory later (DirectX)
                printf("warning: set_gfx_mode(%dx%d): %s\n", width, height, allegro_error);
                panic("failed to open screen with requested resolution");
            }
        }

        // get video memory
        video_page[0] = create_video_bitmap(width, height);
        video_page[1] = create_video_bitmap(width, height);
        if (videobuf_tiles) video_page[2] = create_video_bitmap(width + 2 * WORLD_RENDER_BORDER_SIZE, height + 2 * WORLD_RENDER_BORDER_SIZE);
        else video_page[2] = NULL;

        // error
        if (!video_page[0] || !video_page[1]) panic("unable to acquire sufficient video memory for requested resolution");
        if (!video_page[2] && videobuf_tiles) printf("unable to acquire video memory for tile background; using a memory bitmap instead");

        // set current screen
        video_page_offscreen = 0;
        offscreen = video_page[video_page_offscreen];

        // flip page
        flip_page();
    }
    else { // double buffering
        set_color_depth(32);
        if (set_gfx_mode((windowed ? GFX_AUTODETECT_WINDOWED : GFX_AUTODETECT_FULLSCREEN), width, height, 0, 0) != 0) { // double buffering
            printf("warning: set_gfx_mode(%dx%d): %s\n", width, height, allegro_error);
            panic("failed to open screen with requested resolution");
        }
        video_page[0] = create_bitmap(width, height);
        video_page[1] = NULL;
        video_page[2] = NULL;
        offscreen = video_page[0];
    }
    gui_set_screen(offscreen); // for the allegro gui routines
}

SCREEN::~SCREEN(void)
{
	// release video bitmaps
	if (video_page[0]) destroy_bitmap(video_page[0]);
	if (video_page[1]) destroy_bitmap(video_page[1]);
	if (video_page[2]) destroy_bitmap(video_page[2]);
}

BITMAP* SCREEN::flip_page(void)
{
    if (!doublebuffer) { // page flipping
        // swap pages
        show_video_bitmap(offscreen);
        video_page_offscreen = 1 - video_page_offscreen;
        offscreen = video_page[video_page_offscreen];

        return offscreen;
    }
    else { // double buffering
        blit(video_page[0], screen, 0, 0, 0, 0, w, h);
    }
    gui_set_screen(offscreen); // for the allegro gui routines
    return offscreen;
}

void SCREEN::take_screenshot(const char* filename) {
	if (!save_bitmap(filename, offscreen, NULL)) printf("Screenshot: \'%s\' saved.\n", filename);
	else puts("warning: Screenshot failed!");
}

BITMAP* SCREEN::get_page(void)
{
	return offscreen;
}

BITMAP*& SCREEN::get_page_reference(void)
{
	return offscreen;
}

BITMAP* SCREEN::get_extrapage(void)
{
    return video_page[2];
}

int SCREEN::get_width(void)
{
	return w;
}

int SCREEN::get_height(void)
{
	return h;
}
