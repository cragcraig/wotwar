/* lexgui: skins for the Allegro GUI
 *
 * As totaly bastardized by Craig Harrison
 * so that it can do what he needs it to.
 *
 */

#include "lexgui.h"
#include "allegro/internal/aintern.h"

#define LEX_BMP_COUNT 12
#define LEX_BMP_OFS_BUTTON 0
#define LEX_BMP_OFS_SLIDER 4
#define LEX_BMP_OFS_CHECKBOX 5
#define LEX_BMP_OFS_RADIOBUTTON 6
#define LEX_BMP_OFS_TEXTBOX 7
#define LEX_BMP_OFS_LISTBOX 8
#define LEX_BMP_OFS_DIALOG 11

#define LEX_CALL_BUTTONCALLBACK(d)
static BITMAP *lex__repository[LEX_BMP_COUNT];

/* The currently active skin */
LexSkin lex_skin;

/* very internal update stuff */
void (*lex__screen_update)(BITMAP *);
BITMAP * lex__the_real_screen;
BITMAP * lex__the_fake_screen;
int     (*lex__external_slider_callback)(void *, int);
int      lex__drag_in_progress = 0;
int      lex__needs_update = 0;
int      lex__lastMouseX = -1;
int      lex__lastMouseY = -1;
int      lex__skinLoaded = FALSE;


void update_user_screen();

void loadButtonSkin() {
    char **tokens;
    int    tokenCount;
    int    gridx[4];
    int    gridy[4];
    int    a = 0;
    int    x,y,mode;

    tokens = get_config_argv("button", "gridx", &tokenCount);
    for (a=0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("button", "gridy", &tokenCount);
    for (a=0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }

    tokens                      = get_config_argv("button", "textcol_norm", &tokenCount);
    lex_skin.button.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                      = get_config_argv("button", "textcol_hilite", &tokenCount);
    lex_skin.button.textcolor[1] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                      = get_config_argv("button", "textcol_pressed", &tokenCount);
    lex_skin.button.textcolor[2] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                      = get_config_argv("button", "textcol_disabled", &tokenCount);
    lex_skin.button.textcolor[3] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));

    lex__repository[LEX_BMP_OFS_BUTTON + 0] = load_bitmap(get_config_string("button", "image_norm"   , NULL), NULL);
    lex__repository[LEX_BMP_OFS_BUTTON + 1] = load_bitmap(get_config_string("button", "image_hilite" , NULL), NULL);
    lex__repository[LEX_BMP_OFS_BUTTON + 2] = load_bitmap(get_config_string("button", "image_pressed", NULL), NULL);
    lex__repository[LEX_BMP_OFS_BUTTON + 3] = load_bitmap(get_config_string("button", "image_disabled", NULL), NULL);


    for (mode=0; mode < 4; mode++) {
        a=0;
        for (y=0; y < 3; y++) {
            for (x=0; x < 3; x++) {
                lex_skin.button.background[mode].grid[a] = create_sub_bitmap(
                                lex__repository[LEX_BMP_OFS_BUTTON + mode],
                                gridx[x]             , gridy[y],
                                gridx[x+1]-gridx[x]+1, gridy[y+1]-gridy[y]+1
                                );
                a++;
            }
        }
    }
}


void loadSliderSkin() {
    int    x, y, w, h,o1,o2;
    char **tokens;
    int    tokenCount;

    lex__repository[LEX_BMP_OFS_SLIDER] = load_bitmap(get_config_string("slider", "image"   , NULL), NULL);

    tokens = get_config_argv("slider", "slider_h", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);

    tokens = get_config_argv("slider", "slider_h_ofs", &tokenCount);
    o1 = atoi(tokens[0]); o2 = atoi(tokens[1]);

    lex_skin.slider.hSlider[0] = create_sub_bitmap(lex__repository[LEX_BMP_OFS_SLIDER], x , y, o1      , h);
    lex_skin.slider.hSlider[1] = create_sub_bitmap(lex__repository[LEX_BMP_OFS_SLIDER], o1, y, o2-o1   , h);
    lex_skin.slider.hSlider[2] = create_sub_bitmap(lex__repository[LEX_BMP_OFS_SLIDER], o2, y, w-o2    , h);

    tokens = get_config_argv("slider", "slider_v", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);

    tokens = get_config_argv("slider", "slider_v_ofs", &tokenCount);
    o1 = atoi(tokens[0]); o2 = atoi(tokens[1]);

    lex_skin.slider.vSlider[0] = create_sub_bitmap(lex__repository[LEX_BMP_OFS_SLIDER], x,  y, w, o1);
    lex_skin.slider.vSlider[1] = create_sub_bitmap(lex__repository[LEX_BMP_OFS_SLIDER], x, o1, w, o2 -  o1);
    lex_skin.slider.vSlider[2] = create_sub_bitmap(lex__repository[LEX_BMP_OFS_SLIDER], x, o2, w, h  -  o2);

    tokens = get_config_argv("slider", "handle_v", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    lex_skin.slider.vGrip   = create_sub_bitmap(lex__repository[LEX_BMP_OFS_SLIDER], x, y, w, h);

    tokens = get_config_argv("slider", "handle_h", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    lex_skin.slider.hGrip   = create_sub_bitmap(lex__repository[LEX_BMP_OFS_SLIDER], x, y, w, h);
}

void loadCheckboxSkin()  {
    int    x, y, w,h;
    char **tokens;
		int    tokenCount;

    lex__repository[LEX_BMP_OFS_CHECKBOX] = load_bitmap(get_config_string("checkbox", "image"   , NULL), NULL);


    tokens = get_config_argv("checkbox", "normal", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    lex_skin.checkbox.normal = create_sub_bitmap(lex__repository[LEX_BMP_OFS_CHECKBOX], x , y, w, h);

    tokens = get_config_argv("checkbox", "checked", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    lex_skin.checkbox.checked = create_sub_bitmap(lex__repository[LEX_BMP_OFS_CHECKBOX], x , y, w, h);

    tokens = get_config_argv("checkbox", "disabled", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    lex_skin.checkbox.disabled = create_sub_bitmap(lex__repository[LEX_BMP_OFS_CHECKBOX], x , y, w, h);

    tokens = get_config_argv("checkbox", "disabled_check", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    lex_skin.checkbox.disabled_checked = create_sub_bitmap(lex__repository[LEX_BMP_OFS_CHECKBOX], x , y, w, h);

    tokens                        = get_config_argv("button", "textcol_norm", &tokenCount);
    lex_skin.checkbox.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                        = get_config_argv("button", "textcol_disabled", &tokenCount);
    lex_skin.checkbox.textcolor[1] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
}

void loadRadiobuttonSkin()  {
    int    x, y, w,h;
    char **tokens;
    int    tokenCount;

    lex__repository[LEX_BMP_OFS_RADIOBUTTON] = load_bitmap(get_config_string("radiobutton", "image"   , NULL), NULL);

    tokens = get_config_argv("radiobutton", "normal", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    lex_skin.radiobutton.normal = create_sub_bitmap(lex__repository[LEX_BMP_OFS_RADIOBUTTON], x , y, w, h);

    tokens = get_config_argv("radiobutton", "checked", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    lex_skin.radiobutton.checked = create_sub_bitmap(lex__repository[LEX_BMP_OFS_RADIOBUTTON], x , y, w, h);

    tokens = get_config_argv("radiobutton", "disabled", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    lex_skin.radiobutton.disabled = create_sub_bitmap(lex__repository[LEX_BMP_OFS_RADIOBUTTON], x , y, w, h);

    tokens = get_config_argv("radiobutton", "disabled_check", &tokenCount);
    x = atoi(tokens[0]); y = atoi(tokens[1]);
    w = atoi(tokens[2]); h = atoi(tokens[3]);
    lex_skin.radiobutton.disabled_checked = create_sub_bitmap(lex__repository[LEX_BMP_OFS_RADIOBUTTON], x , y, w, h);

    tokens                        = get_config_argv("button", "textcol_norm", &tokenCount);
    lex_skin.radiobutton.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                        = get_config_argv("button", "textcol_disabled", &tokenCount);
    lex_skin.radiobutton.textcolor[1] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
}

void loadTextboxSkin() {
    char **tokens;
    int    tokenCount;
    int    gridx[4];
    int    gridy[4];
    int    a = 0;
    int    x,y;

    tokens = get_config_argv("textbox", "gridx", &tokenCount);
    for (a=0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("textbox", "gridy", &tokenCount);
    for (a=0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }

    tokens                       = get_config_argv("textbox", "textcol_norm", &tokenCount);
    lex_skin.textbox.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                       = get_config_argv("textbox", "textcol_disabled", &tokenCount);
    lex_skin.textbox.textcolor[1] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));

    lex__repository[LEX_BMP_OFS_TEXTBOX] = load_bitmap(get_config_string("textbox", "image"   , NULL), NULL);



    a=0;
    for (y=0; y < 3; y++) {
        for (x=0; x < 3; x++) {
            lex_skin.textbox.bg.grid[a] = create_sub_bitmap(
                            lex__repository[LEX_BMP_OFS_TEXTBOX],
                            gridx[x]             , gridy[y],
                            gridx[x+1]-gridx[x]+1, gridy[y+1]-gridy[y]+1
                            );
            a++;
        }
    }
}

void loadListboxSkin() {
    char **tokens;
    int    tokenCount;
    int    gridx[4];
    int    gridy[4];
    int    a = 0;
    int    x,y;

    tokens = get_config_argv("listbox", "gridx", &tokenCount);
    for (a=0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("listbox", "gridy", &tokenCount);
    for (a=0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }

    tokens                       = get_config_argv("listbox", "textcol_norm", &tokenCount);
    lex_skin.listbox.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                       = get_config_argv("listbox", "textcol_selected", &tokenCount);
    lex_skin.listbox.textcolor[1] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                       = get_config_argv("listbox", "textbg_selected", &tokenCount);
    lex_skin.listbox.textcolor[2] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                       = get_config_argv("listbox", "textcol_disabled", &tokenCount);
    lex_skin.listbox.textcolor[3] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));

    lex__repository[LEX_BMP_OFS_LISTBOX+0] = load_bitmap(get_config_string("listbox", "image"   , NULL), NULL);
    lex__repository[LEX_BMP_OFS_LISTBOX+1] = load_bitmap(get_config_string("listbox", "vslider" , NULL), NULL);
    lex__repository[LEX_BMP_OFS_LISTBOX+2] = load_bitmap(get_config_string("listbox", "scrollbar" , NULL), NULL);

    a=0;
    for (y=0; y < 3; y++) {
        for (x=0; x < 3; x++) {
            lex_skin.listbox.bg.grid[a] = create_sub_bitmap(
                            lex__repository[LEX_BMP_OFS_LISTBOX],
                            gridx[x]             , gridy[y],
                            gridx[x+1]-gridx[x]+1, gridy[y+1]-gridy[y]+1
                            );
            a++;
        }
    }


    tokens = get_config_argv("listbox", "sb_gridx", &tokenCount);
    for (a=0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("listbox", "sb_gridy", &tokenCount);
    for (a=0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }

    a=0;
    for (y=0; y < 3; y++) {
        for (x=0; x < 3; x++) {
            lex_skin.listbox.scrollbar.grid[a] = create_sub_bitmap(
                            lex__repository[LEX_BMP_OFS_LISTBOX+2],
                            gridx[x]             , gridy[y],
                            gridx[x+1]-gridx[x]+1, gridy[y+1]-gridy[y]+1
                            );
            a++;
        }
    }

    tokens = get_config_argv("listbox", "vscroll_gridx", &tokenCount);
    for (a=0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("listbox", "vscroll_gridy", &tokenCount);
    for (a=0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }
    a=0;
    for (y=0; y < 3; y++) {
        for (x=0; x < 3; x++) {
            lex_skin.listbox.vscroll.grid[a] = create_sub_bitmap(
                            lex__repository[LEX_BMP_OFS_LISTBOX+1],
                            gridx[x]             , gridy[y],
                            gridx[x+1]-gridx[x]+1, gridy[y+1]-gridy[y]+1
                            );
            a++;
        }
    }

}

void loadDialogSkin() {
    char **tokens;
    int    tokenCount;
    int    gridx[4];
    int    gridy[4];
    int    a = 0;
    int    x,y;

    tokens = get_config_argv("dialog", "gridx", &tokenCount);
    for (a=0; a < 4; a++) {
        gridx[a] = atoi(tokens[a]);
    }
    tokens = get_config_argv("dialog", "gridy", &tokenCount);
    for (a=0; a < 4; a++) {
        gridy[a] = atoi(tokens[a]);
    }

    lex__repository[LEX_BMP_OFS_DIALOG] = load_bitmap(get_config_string("dialog", "image"   , NULL), NULL);

    lex_skin.dialog.titleAlign = get_config_int("dialog", "align", 1);

    a=0;
    for (y=0; y < 3; y++) {
        for (x=0; x < 3; x++) {
            lex_skin.dialog.bg.grid[a] = create_sub_bitmap(
                            lex__repository[LEX_BMP_OFS_DIALOG],
                            gridx[x]             , gridy[y],
                            gridx[x+1]-gridx[x]+1, gridy[y+1]-gridy[y]+1
                            );
            a++;
        }
    }

    tokens                       = get_config_argv("dialog", "textcol_norm", &tokenCount);
    lex_skin.dialog.textcolor[0] = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
    tokens                       = get_config_argv("dialog", "bgcolor", &tokenCount);
    lex_skin.dialog.bgcol        = makecol(atoi(tokens[0]),atoi(tokens[1]),atoi(tokens[2]));
}






void drawSkinnedRect(BITMAP*dst, LexSkinnedRect *skin, int x, int y,int w, int h) {

    BITMAP **grid = skin->grid;

    int w0 = grid[0]->w;
    int w1 = w - grid[0]->w -grid[2]->w;
    int w2 = grid[2]->w;
    int h0 = grid[0]->h;
    int h1 = h - grid[0]->h - grid[6]->h;
    int h2 = grid[6]->h;

    int cx,cy;

    cx = x; cy = y;
    masked_blit(grid[0], dst, 0, 0, cx, cy,grid[0]->w,grid[0]->h);
    cy += h0;
    masked_stretch_blit(grid[3], dst, 0, 0, grid[3]->w,grid[3]->h,cx, cy,w0,h1);
    cy += h1;
    masked_blit(grid[6], dst, 0, 0, cx, cy,grid[6]->w,grid[6]->h);

    cx += w0;
    cy  = y;
    masked_stretch_blit(grid[1], dst, 0, 0, grid[1]->w,grid[1]->h,cx, cy,w1,h0);
    cy += h0;
    masked_stretch_blit(grid[4], dst, 0, 0, grid[4]->w,grid[4]->h,cx, cy,w1,h1);
    cy += h1;
    masked_stretch_blit(grid[7], dst, 0, 0, grid[7]->w,grid[7]->h,cx, cy,w1,h2);

    cx += w1;
    cy  = y;
    masked_blit(grid[2], dst, 0, 0, cx, cy,grid[2]->w,grid[2]->h);
    cy += h0;
    masked_stretch_blit(grid[5], dst, 0, 0, grid[5]->w,grid[5]->h,cx, cy,w2,h1);
    cy += h1;
    masked_blit(grid[8], dst, 0, 0, cx, cy,grid[8]->w,grid[7]->h);
}


int lex_load_skin(const char* skinname) {

    int a;

    if (lex__skinLoaded) {
        lex_gui_shutdown();
    } else {
        lex__screen_update = NULL;
        lex__external_slider_callback = NULL;
    }
    memset(&lex_skin, 0, sizeof(lex_skin));

    push_config_state();
    set_config_file(skinname);

    loadButtonSkin();
    loadSliderSkin();
    loadCheckboxSkin();
    loadRadiobuttonSkin();
    loadTextboxSkin();
    loadListboxSkin();
    loadDialogSkin();

    pop_config_state();

    for (a=0; a < LEX_BMP_COUNT;a++) {
        if (lex__repository[a] == NULL) {
            allegro_message("Unable to load skin no:%i", a);
            return FALSE;
        }
    }
    lex__skinLoaded = TRUE;

    return TRUE;
}

void lex_gui_shutdown(void) {
    int a,b;

	a = b = 0;

    for (a=0; a < 3; a++) {
        destroy_bitmap(lex_skin.slider.vSlider[a]);
		destroy_bitmap(lex_skin.slider.hSlider[a]);
    }

    for (a=0; a < 4; a++) {
        for (b=0; b< 9; b++) {
            destroy_bitmap(lex_skin.button.background[a].grid[b]);
            lex_skin.button.background[a].grid[b] = NULL;
        }
    }

    destroy_bitmap(lex_skin.slider.hGrip);
    destroy_bitmap(lex_skin.slider.vGrip);

    destroy_bitmap(lex_skin.checkbox.normal);
    destroy_bitmap(lex_skin.checkbox.checked);
    destroy_bitmap(lex_skin.checkbox.disabled);
    destroy_bitmap(lex_skin.checkbox.disabled_checked);

    destroy_bitmap(lex_skin.radiobutton.normal);
    destroy_bitmap(lex_skin.radiobutton.checked);
    destroy_bitmap(lex_skin.radiobutton.disabled);
    destroy_bitmap(lex_skin.radiobutton.disabled_checked);

    for (b=0; b< 9; b++) {
        destroy_bitmap(lex_skin.textbox.bg.grid[b]);
        destroy_bitmap(lex_skin.listbox.bg.grid[b]);
        destroy_bitmap(lex_skin.listbox.scrollbar.grid[b]);
        destroy_bitmap(lex_skin.listbox.vscroll.grid[b]);
        destroy_bitmap(lex_skin.dialog.bg.grid[b]);
    }

    for (a=0; a < LEX_BMP_COUNT; a++) {
        destroy_bitmap(lex__repository[a]);
        lex__repository[a] = NULL;
    }

    memset(&lex_skin, 0, sizeof(lex_skin));
}

int lex_button_proc(int msg, DIALOG *d, int c) {

    int rtm = 0;
    int col = 0;
    int ofs = 0;
    int ret = D_O_K;

    if (msg == MSG_DRAW) {
        scare_mouse();
        rectfill(gui_get_screen(), d->x, d->y, d->x + d->w, d->y+d->h, d->bg);

        if (d->flags & D_DISABLED) {
            drawSkinnedRect(gui_get_screen(), &lex_skin.button.background[3], d->x, d->y, d->w, d->h);
            col = lex_skin.button.textcolor[3];
        } else if (d->flags & D_SELECTED) {
            drawSkinnedRect(gui_get_screen(), &lex_skin.button.background[2], d->x, d->y, d->w, d->h);
            col = lex_skin.button.textcolor[2];
            ofs = 1;
        } else if (d->flags & D_GOTMOUSE) {
            drawSkinnedRect(gui_get_screen(), &lex_skin.button.background[1], d->x, d->y, d->w, d->h);
            col = lex_skin.button.textcolor[1];
        } else {
            drawSkinnedRect(gui_get_screen(), &lex_skin.button.background[0], d->x, d->y, d->w, d->h);
            col = lex_skin.button.textcolor[0];
        }
	    gui_textout_ex(gui_get_screen(), d->dp, d->x+d->w/2+ofs, d->y+d->h/2-text_height(font)/2+ofs, col, -1, TRUE);
        lex__needs_update = TRUE;
        unscare_mouse();
    } else {
        ret =  d_button_proc(msg,d,c);
        if (ret == D_CLOSE && d->dp2 != NULL) {
            ret = (*(lex_buttonCallback)(d->dp2))(d->d1);
            object_message(d, MSG_DRAW, 0);
        }
        if (msg == MSG_IDLE && (lex__needs_update || mouse_x != lex__lastMouseX || mouse_y != lex__lastMouseY)) {
            update_user_screen();
            lex__needs_update = FALSE;
        }
    }
    return ret;
}

int lex_slider_proc(int msg, DIALOG *d, int c) {
    int w   = 0;
    int h   = 0;
    int x,y;

    int ret = D_O_K;

	static int watchdog = 0;

	watchdog++;
    /*
	if (watchdog == 1) {
		lex__external_slider_callback = d->dp2;
		d->dp2 = reroute_slider_proc;
	}
    */

    if (msg == MSG_DRAW) {
        if (d->w >= d->h) {
            rectfill(gui_get_screen(), d->x, d->y, d->x + d->w, d->y+d->h, d->bg);
            /* horiz */
            x = d->x;
            y = d->y + (d->h- lex_skin.slider.hSlider[0]->h)/2;
            masked_blit(lex_skin.slider.hSlider[0], gui_get_screen(), 0, 0,  x, y, lex_skin.slider.hSlider[0]->w, lex_skin.slider.hSlider[0]->h);
            w   = d->w -lex_skin.slider.hSlider[0]->w - lex_skin.slider.hSlider[2]->w;
            x+= lex_skin.slider.hSlider[0]->w;

            masked_stretch_blit(
                    lex_skin.slider.hSlider[1], gui_get_screen(),
                    0, 0,  lex_skin.slider.hSlider[1]->w, lex_skin.slider.hSlider[1]->h,
                    x, y, w, lex_skin.slider.hSlider[1]->h);

            x+=w;
            masked_blit(lex_skin.slider.hSlider[2], gui_get_screen(), 0, 0,  x, y, lex_skin.slider.hSlider[2]->w, lex_skin.slider.hSlider[2]->h);

            x  = d->x + ((d->w-lex_skin.slider.hGrip->w) * d->d2)/d->d1;
            y = d->y + (d->h - lex_skin.slider.hGrip->h)/2;
            masked_blit(lex_skin.slider.hGrip, gui_get_screen(), 0, 0,  x, y, lex_skin.slider.hGrip->w, lex_skin.slider.hGrip->h);

            lex__needs_update = TRUE;
        } else {
            rectfill(gui_get_screen(), d->x, d->y, d->x + d->w, d->y+d->h, d->bg);
            /* vertic */
            x = d->x+ (d->w- lex_skin.slider.vSlider[0]->w)/2;
            y = d->y;
            masked_blit(lex_skin.slider.vSlider[0], gui_get_screen(), 0, 0,  x, y, lex_skin.slider.vSlider[0]->w, lex_skin.slider.vSlider[0]->h);
            h   = d->h - lex_skin.slider.vSlider[0]->h - lex_skin.slider.vSlider[2]->h;
            y  += lex_skin.slider.vSlider[0]->h;

            masked_stretch_blit(
                    lex_skin.slider.vSlider[1], gui_get_screen(),
                    0, 0,  lex_skin.slider.vSlider[1]->w, lex_skin.slider.vSlider[1]->h,
                    x, y,  lex_skin.slider.vSlider[1]->w, h);

            y+=h;
            masked_blit(lex_skin.slider.vSlider[2], gui_get_screen(), 0, 0,  x, y, lex_skin.slider.vSlider[2]->w, lex_skin.slider.vSlider[2]->h);

            y = d->y + d->h - (((d->h-lex_skin.slider.vGrip->h) * d->d2)/d->d1)-lex_skin.slider.vGrip->h;
            x = d->x + (d->w - lex_skin.slider.vGrip->w)/2;
            if (lex_skin.slider.vGrip->w % 2 !=0) {
                x++;
            }
            masked_blit(lex_skin.slider.vGrip, gui_get_screen(), 0, 0,  x, y, lex_skin.slider.vGrip->w, lex_skin.slider.vGrip->h);
        }
        //textprintf(gui_get_screen(), font,10, 10, makecol(255,255,255), "%i", d->d2);
    } else {
        ret = d_slider_proc(msg,d,c);
        if (msg == MSG_IDLE && (lex__needs_update || mouse_x != lex__lastMouseX || mouse_y != lex__lastMouseY)) {
            update_user_screen();
            lex__needs_update = FALSE;
        }
    }
    /*
	if (watchdog == 1) {
		d->dp2 = lex__external_slider_callback;
	}
    */

	watchdog--;
    return ret;
}

int lex_check_proc(int msg, DIALOG *d, int c) {
    BITMAP *box = NULL;
    int     x, y;
    int     tx, ty, l;
    int     rtm = 0;
    int     col = 0;
    int     ret = D_O_K;

    if (msg == MSG_DRAW) {
        rectfill(gui_get_screen(), d->x, d->y, d->x + d->w, d->y+d->h, d->bg);
        if (d->flags & D_SELECTED) {
            if (d->flags & D_DISABLED) {
                box = lex_skin.checkbox.disabled_checked;
            } else {
                box = lex_skin.checkbox.checked;
            }
        } else if (d->flags & D_DISABLED) {
            box = lex_skin.checkbox.disabled;
        } else {
            box = lex_skin.checkbox.normal;
        }

        if (d->flags & D_DISABLED) {
            col = lex_skin.checkbox.textcolor[1];
        } else {
            col = lex_skin.checkbox.textcolor[0];
        }

        if (d->dp != NULL) {
            l = gui_strlen(d->dp);
        } else {
            l = 0;
        }

        if (d->d1 != 0) {
            x  = d->x;
            tx = x + box->w + box->w/2;
        } else {
            x  = d->x + d->w - box->w;
            tx = x - box->w/2 - l;
        }
        y  = d->y + (d->h - box->h)/ 2;
        ty = d->y + (d->h - text_height(font)) / 2;

        masked_blit(box, gui_get_screen(), 0, 0, x, y, box->w, box->h);
        if (d->dp != NULL) {
            gui_textout_ex(gui_get_screen(), d->dp, tx, ty, col, -1, FALSE);
        }
        lex__needs_update = TRUE;

    } else {
        ret = d_check_proc(msg, d, c);
        if (msg == MSG_IDLE && (lex__needs_update || mouse_x != lex__lastMouseX || mouse_y != lex__lastMouseY)) {
            update_user_screen();
            lex__needs_update = FALSE;
        }
    }
    return ret;
}

int lex_radio_proc(int msg, DIALOG *d, int c) {
    BITMAP *box = NULL;
    int     x, y;
    int     tx, ty, l;
    int     rtm = 0;
    int     col = 0;
    int     ret = 0;

    if (msg == MSG_DRAW) {
        rectfill(gui_get_screen(), d->x, d->y, d->x + d->w, d->y+d->h, d->bg);
        if (d->flags & D_SELECTED) {
            if (d->flags & D_DISABLED) {
                box = lex_skin.radiobutton.disabled_checked;
            } else {
                box = lex_skin.radiobutton.checked;
            }
        } else if (d->flags & D_DISABLED) {
            box = lex_skin.radiobutton.disabled;
        } else {
            box = lex_skin.radiobutton.normal;
        }

        if (d->flags & D_DISABLED) {
            col = lex_skin.radiobutton.textcolor[1];
        } else {
            col = lex_skin.radiobutton.textcolor[0];
        }

        if (d->dp != NULL) {
            l = gui_strlen(d->dp);
        } else {
            l = 0;
        }

        if (d->d2 != 0) {
            x  = d->x;
            tx = x + box->w + box->w/2;
        } else {
            x  = d->x + d->w - box->w;
            tx = x - box->w/2 - l;
        }
        y  = d->y + (d->h - box->h)/ 2;
        ty = d->y + (d->h - text_height(font)) / 2;

        masked_blit(box, gui_get_screen(), 0, 0, x, y, box->w, box->h);
        if (d->dp != NULL) {
            gui_textout_ex(gui_get_screen(), d->dp, tx, ty, col, -1, FALSE);
        }

        lex__needs_update = TRUE;
    } else {
        ret = d_radio_proc(msg, d, c);
        if (msg == MSG_IDLE && (lex__needs_update || mouse_x != lex__lastMouseX || mouse_y != lex__lastMouseY)) {
            update_user_screen();
            lex__needs_update = FALSE;
        }
    }
    return ret;
}

int lex_edit_proc(int msg, DIALOG *d, int c) {
    BITMAP *box = NULL;
    int     x;
    int     tx, ty, l;
    int     rtm = 0;
    int     col = 0;
    char*   start;
    char*   text;
    char    hack;
    int     cl, cr, cb, ct;
    int     lb, rb;
    int     ret = D_O_K;

    if (msg == MSG_DRAW) {
        rectfill(gui_get_screen(), d->x, d->y, d->x + d->w, d->y+d->h, d->bg);
        drawSkinnedRect(gui_get_screen(), &lex_skin.textbox.bg, d->x, d->y, d->w, d->h);

        if (d->flags & D_DISABLED) {
            col = lex_skin.textbox.textcolor[1];
        } else {
            col = lex_skin.textbox.textcolor[0];
        }

        lb = lex_skin.textbox.bg.grid[0]->w;
        rb = lex_skin.textbox.bg.grid[2]->w;
        tx = d->x + lb;
        ty = d->y + (d->h - text_height(font))/2;


        text  = d->dp;
        start = text;

        if (gui_get_screen()->clip) {
            cl = gui_get_screen()->cl;
            ct = gui_get_screen()->ct;
            cr = gui_get_screen()->cr;
            cb = gui_get_screen()->cb;
        } else {
            cl=ct=0;
            cr=gui_get_screen()->w;
            cb=gui_get_screen()->h;
        }
        set_clip_rect(gui_get_screen(), tx, ty, d->x+d->w-rb, ty + text_height(font));

        hack        = text[d->d2];
        text[d->d2] = '\0';
        l = text_length(font, text);
        text[d->d2] = hack;

        if (l > d->w-lb-rb) {
            tx += ((d->w-lb-rb) - l);
        }
        gui_textout_ex(gui_get_screen(), start, tx, ty, col, -1, FALSE);


        if (d->flags & D_GOTFOCUS) {
            hack        = text[d->d2];
            text[d->d2] = '\0';
            x = tx + text_length(font, text);
            vline(gui_get_screen(), x, ty, ty + text_height(font), col);
            text[d->d2] = hack;
        }
        set_clip_rect(gui_get_screen(), cl, ct, cr, cb);

        lex__needs_update = TRUE;
    } else {
        ret= d_edit_proc(msg, d, c);
        if (msg == MSG_IDLE && (lex__needs_update || mouse_x != lex__lastMouseX || mouse_y != lex__lastMouseY)) {
            update_user_screen();
            lex__needs_update = FALSE;
        }
    }
    return ret;
}

int lex_list_proc(int msg, DIALOG *d, int c) {
    BITMAP *box = NULL;

    static int ignoreRedraw = FALSE;

    int itemCount     = 0;
    int firstItem     = d->d2;
    int lastItem      = 0;
    int selectedItem  = d->d1;
    int x,y,delta;
    int a, col;
    int w, h          = 0;
    int rtm           = 0;
    int cl, cr, cb, ct;
    int th            = text_height(font);

    int vscroll = 0;
    int sliderh = 10;
    int slidery = 0;
    int ret = D_O_K;

    (*(getfuncptr)d->dp)(-1, &itemCount);

    w = d->w - lex_skin.listbox.bg.grid[0]->w - lex_skin.listbox.bg.grid[2]->w;
    h = d->h - lex_skin.listbox.bg.grid[1]->h - lex_skin.listbox.bg.grid[7]->h;
    lastItem = MIN(itemCount-1, firstItem + h / text_height(font));

    if (msg == MSG_DRAW) {
        if (ignoreRedraw) {
            return D_O_K;
        }
        rectfill(gui_get_screen(), d->x, d->y, d->x + d->w, d->y+d->h, d->bg);
        drawSkinnedRect(gui_get_screen(), &lex_skin.listbox.bg, d->x, d->y, d->w, d->h);

        (*(getfuncptr)d->dp)(-1, &itemCount);
        vscroll = (h/th) < (itemCount-1);
        if (vscroll) {
            w = d->w - 17 - lex_skin.listbox.bg.grid[0]->w;
            drawSkinnedRect(gui_get_screen(), &lex_skin.listbox.scrollbar, d->x+d->w-15, d->y+1, 14, d->h-2);
            sliderh = MAX(((d->h-2)* (h / th)) / itemCount, lex_skin.listbox.bg.grid[0]->h*2);
            slidery = ((d->h-2-sliderh) * firstItem) / (itemCount);
            slidery+= d->y+1;
            drawSkinnedRect(gui_get_screen(), &lex_skin.listbox.vscroll, d->x+d->w-13, slidery, 11, sliderh);
        }

        if (gui_get_screen()->clip) {
            cl = gui_get_screen()->cl;
            ct = gui_get_screen()->ct;
            cr = gui_get_screen()->cr;
            cb = gui_get_screen()->cb;
        } else {
            cl=ct=0;
            cr=gui_get_screen()->w;
            cb=gui_get_screen()->h;
        }
        x = d->x + lex_skin.listbox.bg.grid[0]->w;
        y = d->y + lex_skin.listbox.bg.grid[0]->h;
        set_clip_rect(gui_get_screen(), x,y, x+w, y+h);


        if (d->flags & D_DISABLED) {
            col = lex_skin.listbox.textcolor[3];
            for (a=firstItem; a < lastItem; a++) {
                textout_ex(gui_get_screen(), font, (*(getfuncptr)d->dp)(a, 0), x, y, col, -1);
                y += text_height(font);
            }
        } else {
            for (a=firstItem; a <= lastItem; a++) {
                if (a==d->d1) {
                    col = lex_skin.listbox.textcolor[1];
                    rectfill(gui_get_screen(), x, y, x+w, y+text_height(font)-1, lex_skin.listbox.textcolor[2]);
                } else {
                    col = lex_skin.listbox.textcolor[0];
                }
                textout_ex(gui_get_screen(), font, (*(getfuncptr)d->dp)(a, 0), x, y, col, -1);
                y += text_height(font);
            }
        }

        set_clip_rect(gui_get_screen(), cl, ct, cr, cb);
        lex__needs_update = TRUE;
    } else if (msg == MSG_CLICK) {

        x = d->x + lex_skin.listbox.bg.grid[0]->w;
        y = d->y + lex_skin.listbox.bg.grid[0]->h;

        sliderh = MAX(((d->h-2)* (h / th)) / itemCount, lex_skin.listbox.bg.grid[0]->h*2);
        //sliderh = ((d->h-2)* (h / th)) / itemCount;
        slidery = ((d->h-2-sliderh) * firstItem) / (itemCount);
        slidery+= d->y+1;

        if (mouse_x > (d->x + d->w - 14) && mouse_x < (d->x+d->w-1)) {
            // Ok, scroll bar
            if (mouse_y >= slidery && mouse_y < slidery + sliderh) {
                delta= mouse_y - slidery;
                while (mouse_b) {
                    a  = mouse_y - delta - d->y -1;
                    a *= itemCount;
                    a /= (d->h-2);
                    a  = MID(0, a, itemCount- h/th);

                    if (a != d->d2) {
                        d->d2 = a;
                        scare_mouse();
                        object_message(d, MSG_DRAW, 0);
                        unscare_mouse();
                    }

                    slidery = ((d->h-2) * firstItem) / (itemCount);
                    slidery+= d->y+1;

                    /* let other objects continue to animate */
                    broadcast_dialog_message(MSG_IDLE, 0);
                }
            } else if (mouse_y < slidery) {
                a = d->d2 - (h/th)+1;
                a = MID(0, a, itemCount- h/th);


                d->d2 = a;
                scare_mouse();
                object_message(d, MSG_DRAW, 0);
                unscare_mouse();

                while (mouse_b) {
                }
            } else if (mouse_y > slidery + sliderh) {
                a = d->d2 + (h/th)-1;
                a = MID(0, a, itemCount- h/th);
                d->d2 = a;

                scare_mouse();
                object_message(d, MSG_DRAW, 0);
                unscare_mouse();

                while (mouse_b) {
                    /* let other objects continue to animate */
                    broadcast_dialog_message(MSG_IDLE, 0);
                }
            }
        } else if (mouse_x >= x && mouse_x < x+w && mouse_y >= y && mouse_y < y+h) {

            while (mouse_b) {
                a = firstItem + (mouse_y-y) / text_height(font);

                if (a <= lastItem && a != selectedItem) {
                    d->d1 = selectedItem = a;
                    scare_mouse();
                    object_message(d, MSG_DRAW, 0);
                    unscare_mouse();
                }
                /* let other objects continue to animate */
                broadcast_dialog_message(MSG_IDLE, 0);
            }
        }
    } else {
        ignoreRedraw = (msg == MSG_GOTFOCUS || msg == MSG_LOSTFOCUS);
        ret =  d_list_proc(msg, d, c);

        if (ret == D_USED_CHAR) {
            if (d->d1 < d->d2) {
                if (d->d1 > 0) {
                    d->d1 = d->d2;
                }
            } else if (d->d1 > d->d2 + h/th -1) {
                d->d2 = d->d1 - h/th + 1;
            }
        }

        if (msg == MSG_IDLE && (lex__needs_update || mouse_x != lex__lastMouseX || mouse_y != lex__lastMouseY)) {
            update_user_screen();
            lex__needs_update = FALSE;
        }
    }
    return ret;
}

void update_user_screen() {
    if (lex__screen_update != NULL) {
        gui_set_screen(lex__the_real_screen);
        lex__screen_update(lex__the_fake_screen);
        gui_set_screen(lex__the_fake_screen);

        lex__lastMouseX = mouse_x;
        lex__lastMouseY = mouse_y;
    }
}

int lex_dialog_proc(int msg, DIALOG *d, int c) {

    int rtm;
    int dx, dy,x,y;

    if (msg == MSG_START) {
        d->dp2 = create_bitmap(gui_get_screen()->w, gui_get_screen()->h);
        scare_mouse();
        blit(gui_get_screen(), d->dp2, 0, 0, 0, 0, gui_get_screen()->w, gui_get_screen()->h);
        unscare_mouse();
    } else if (msg == MSG_END) {
        scare_mouse();
        blit(d->dp2, gui_get_screen(), 0, 0, 0, 0, gui_get_screen()->w, gui_get_screen()->h);
        unscare_mouse();
        destroy_bitmap(d->dp2);
    } else if (msg == MSG_DRAW) {
        //rectfill(gui_get_screen(), d->x, d->y, d->x + d->w-1, d->y+d->h-1, d->bg);
        blit(d->dp2, gui_get_screen(),d->x, d->y, d->x, d->y, d->w, d->h);
        drawSkinnedRect(gui_get_screen(), &lex_skin.dialog.bg, d->x, d->y, d->w, d->h);

        switch (lex_skin.dialog.titleAlign) {
            case 0:
                textout_ex(gui_get_screen(), font, d->dp, d->x + lex_skin.dialog.bg.grid[0]->w, d->y + (lex_skin.dialog.bg.grid[1]->h - text_height(font))/2, lex_skin.dialog.textcolor[0], -1);
                break;
            case 1:
                textout_centre_ex(gui_get_screen(), font, d->dp, d->x + d->w/2, d->y + (lex_skin.dialog.bg.grid[1]->h - text_height(font))/2, lex_skin.dialog.textcolor[0], -1);
                break;
            case 2:
                textout_right_ex(gui_get_screen(), font, d->dp, d->x + d->w - lex_skin.dialog.bg.grid[2]->w, d->y + (lex_skin.dialog.bg.grid[1]->h - text_height(font))/2, lex_skin.dialog.textcolor[0], -1);
                break;
        }
        lex__needs_update = TRUE;
    } else if (msg == MSG_CLICK && !d->d1) {
        if (mouse_y < d->y + lex_skin.dialog.bg.grid[1]->h) {
            dx = mouse_x - d->x;
            dy = mouse_y - d->y;

            lex__drag_in_progress = TRUE;
            while (mouse_b) {
                x = mouse_x - dx;
                y = mouse_y - dy;

                if (x!= d->x || y!= d->y) {
                    scare_mouse();
                    vsync();
                    blit(d->dp2, gui_get_screen(),d->x, d->y, d->x, d->y, d->w, d->h);
                    blit(gui_get_screen(), d->dp2,   x,    y,    x,    y, d->w, d->h);
                    position_dialog(active_dialog, x, y);
                    broadcast_dialog_message(MSG_DRAW, 0);
                    unscare_mouse();
                }
                broadcast_dialog_message(MSG_IDLE, 0);
            }
            lex__drag_in_progress = FALSE;
        }
    } else if (msg == MSG_IDLE && (lex__needs_update || mouse_x != lex__lastMouseX || mouse_y != lex__lastMouseY)) {
        update_user_screen();
        lex__needs_update = FALSE;
    }
    return D_O_K;
}

/*
int reroute_slider_proc(void *dp3, int d2) {
    int ret = 0;

    if (lex__external_slider_callback != NULL) {
        ret = lex__external_slider_callback(dp3, d2);
    }


    return ret;
}
*/

void lex_init_dialog(DIALOG *dialog, int redraw) {
    int cur = 0;
    while (dialog[cur].proc != NULL) {
        dialog[cur].bg = lex_skin.dialog.bgcol;
        cur++;
    }
    if (redraw) {
        broadcast_dialog_message(MSG_DRAW, 0);
    }
}

int lex_do_dialog(DIALOG *dialog, int focus_obj, BITMAP *buffer, void (*proc)(BITMAP*)){
    BITMAP *mouse_screen = _mouse_screen;
    int screen_count = _gfx_mode_set_count;
    int ret = 0;
    void *player;
    int cur=0;

    static int watchdog = 0;

    watchdog++;
    if (watchdog == 1) {
        lex__the_real_screen = gui_get_screen();
        lex__the_fake_screen = buffer;
        lex__screen_update   = proc;

        gui_set_screen(lex__the_fake_screen);
    }

    lex_init_dialog(dialog, FALSE);


    player = init_dialog(dialog, focus_obj);
    while (update_dialog(player)) {
        update_user_screen();
    }

    ret =  shutdown_dialog(player);
    if (watchdog > 1) {
        update_user_screen();
    }
    if (watchdog == 1) {
        lex__screen_update = NULL;
        gui_set_screen(lex__the_real_screen);
    }
    watchdog--;

    return ret;
}
int lex_set_screenupdate(void (*proc)(BITMAP*))
{
	lex__screen_update = proc;
}
