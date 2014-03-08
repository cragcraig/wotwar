#include "wotwar.h"
#include "lexgui.h"
#include <vector>
#include <string>
#include <cstring>

// lex gui bastardization stuff
extern GLOBAL_CONFIG global_config;
static SCREEN * screen_obj;
static BITMAP * mouse_cursor;
static EDITOR_TOOL * etool;
static GAME * game_obj;
void draw_screenobj(BITMAP* buffer);
void draw_mouse(BITMAP* buffer, BITMAP* mouse);
vector<string> maps;

// GUI object, hides the nastly lex gui implementation from the rest of the program

GUI::GUI(const char * title, SCREEN& screen) : game(NULL), dialog(NULL), screen(screen), title(title), buffer(screen.get_page_reference()), but_flag(0), already_pressed(false), in_mainmenu(true)
{
    if (!lex_load_skin("data/gui/gui.skin"))
        panic("unable to load GUI skin 'data/gui/gui.skin'");

    // lex gui stuff
    screen_obj = &screen;
    mouse_cursor = ui_data.mouse_cursor;
    lex_set_screenupdate(NULL);
}

GUI::~GUI(void)
{
    if (game) delete game;
    lex_gui_shutdown();
}

void GUI::update(bool draw)
{
    but_flag = 0;

    // check for failed game
    if (game && game->is_failed()) end_game();

    // draw game
    if (game) {
        // update game
        game->update(draw);

        // editor menus
        if (draw && game->is_editmode()) {
            draw_buttons();
            textout_centre_ex(buffer, font, "Map Editor", screen.get_width()/2, 10, makecol(220,220,220), -1);
            if (but_flag == 1 || key[KEY_T]) tile_editor();
            if (but_flag == 2 || key[KEY_O]) object_editor();
            if (but_flag == 3 || key[KEY_B]) building_editor();
            if (but_flag == 4 || key[KEY_U]) unit_editor();
            if (but_flag == 5 || key[KEY_P]) civ_editor();
            if (but_flag == 6 || key[KEY_A]) save_dialog();
            if (but_flag == 7) end_game();

        // check if fallen too many frames behind
        } else if (!get_fpsdebt() > MAX_FPSDEBT) {
            end_game();
            message("Notification", "Unable to maintain enough frames per second");
        }
    }

    // run dialog
    if (dialog && !but_flag) run_dialog(dialog, center_it);

    // main menu
    if (!game) in_mainmenu = true;
    if (in_mainmenu && draw) main_menu();

    // Mouse
	if (draw) draw_mouse(buffer, ui_data.mouse_cursor);
}

void GUI::display_dialog(DIALOG * d, bool center)
{
    dialog = d;
    center_it = center;
}

int GUI::run_dialog(DIALOG * d, bool center, int focus)
{
    // center
    if (center) {
        int i = 1, x, y;
        x = d[0].x;
        y = d[0].y;
        d[0].x = screen.get_width()/2 - d[0].w/2;
        d[0].y = screen.get_height()/2 - d[0].h/2;
        while (d[i].proc != NULL) {
            d[i].x = d[i].x - x + d[0].x;
            d[i].y = d[i].y - y + d[0].y;

            // call slider callback function on start
            if (d[i].proc == lex_slider_proc && d[i].dp2)
                (*((int (*)(void* dp3, int d2))d[i].dp2))(d[i].dp3, d[i].d2);
            i++;
        }
    }

    int r;

    // messy but easiest way to run lex gui with all features
    BITMAP* buf = create_bitmap(screen.get_width(), screen.get_height());
    blit(screen_obj->get_page(), buf, 0, 0, 0, 0, screen_obj->get_width(), screen_obj->get_height());
    r = lex_do_dialog(d, focus, buf, draw_screenobj);
    destroy_bitmap(buf);
    clear_speedcounter();
    dialog = NULL;

    return r;
}

void GUI::new_game(void)
{
    const char * fname = load_dialog();
    if (!fname) return;
    end_game();
    if (!game) game = new GAME(buffer, screen, ui_data, *this, 0, 1, fname, 100, false, true, true);
    if (!game) panic("failed to acquire memory for game");
    in_mainmenu = false;
}

void GUI::join_game(void)
{
    string host = text_dialog("Host IP");
    if (!host.length()) return;
    end_game();
    if (!game) game = new GAME(buffer, screen, ui_data, *this, 0, 1, NULL, 100, false, false, true, host.c_str());
    if (!game) panic("failed to acquire memory for game");
    in_mainmenu = false;
}

void GUI::new_backgroundgame(void)
{
    in_mainmenu = true;
    if (!game) game = new GAME(buffer, screen, ui_data, *this, 2, 1, NULL, 50, false, false, false);
    if (!game) panic("failed to acquire memory for background game");
}

bool GUI::is_ingame(void)
{
    return game;
}

#define GUI_RALIGN(x, txt) (screen.get_width() - gui_strlen(txt) - (x) - 5)
void GUI::main_menu(void)
{
    // background game
    if (game) game->generate_battle();
    else {
        new_backgroundgame();
        if (game) {
            game->message("Game Engine by Craig");
            game->message(global_config.GAME_CREDITS);
        }
    }

    // draw logo
    masked_blit(ui_data.logo, buffer, 0, 0, buffer->w/2 - ui_data.logo->w/2, ui_data.logo->h/4, ui_data.logo->w, ui_data.logo->h);

    // buttons
    if (!buttons.size()) {
        add_button(10, buffer->h - 40, "Host Game", 1);
        add_button(-1, buffer->h - 40, "Join Game", 2);
        add_button(-1, buffer->h - 40, "Map Editor", 3);
        add_button(GUI_RALIGN(30, "Exit"), buffer->h - 40, "Exit", 4);
    }

    is_over(mouse_x, mouse_y);
    draw_buttons();
    if (but_flag) clear_buttons();

    switch (but_flag) {
        case 1: // new game
            new_game();
            break;

        case 2: // join game
            join_game();
            break;

        case 3: // map editor
            map_editor();
            break;

        case 4: // exit
            exit_game();
            break;
    }
}

void GUI::new_gameedit(int size, int players, bool load)
{
    const char * fname = NULL;

    if (load) {
        fname = load_dialog();
        if (!fname) return;
    }

    end_game();
    game = new GAME(buffer, screen, ui_data, *this, players, 0, fname, size, true, false, false);
    if (!game) panic("failed to acquire memory for game");
    in_mainmenu = false;
    etool = game->get_etool();
    game_obj = game;

    add_button(10, 10, "&Tile brush", 1);
    add_button(-1, 10, "&Objects", 2);
    add_button(-1, 10, "&Buildings", 3);
    add_button(-1, 10, "&Units", 4);
    add_button(GUI_RALIGN(20, "&Player Settings"), 10, "&Player Settings", 5);
    add_button(GUI_RALIGN(20, "S&ave Map"), -1, "S&ave Map", 6);
    add_button(GUI_RALIGN(20, "Exit Editor"), -1, "Exit Editor", 7);
}

void GUI::end_game(void)
{
    if (game) {
        delete game;
        game = NULL;
        clear_buttons();
        clear_keybuf();
    }
}

bool GUI::is_over(int x, int y)
{
    list<BUTTON>::iterator p;
    for (p=buttons.begin(); p!=buttons.end(); p++)
        if (p->is_over(x, y, already_pressed)) return true;

    return false;
}

void GUI::draw_buttons(void)
{
    list<BUTTON>::iterator p;
    for (p=buttons.begin(); p!=buttons.end(); p++)
        p->draw(buffer);

    if (mouse_b & 1) already_pressed = true;
    else already_pressed = false;
}

void GUI::add_button(int x, int y, const char* txt, int val)
{
    if (x < 0) x = buttons.back().x + buttons.back().w + 8;
    if (y < 0) y = buttons.back().y + buttons.back().h + 5;

    buttons.push_back(BUTTON(x, y, txt, &but_flag, val));
}

void GUI::clear_buttons(void)
{
    buttons.clear();
}

BUTTON::BUTTON(int x, int y, const char* txt, int* flag, int val) : x(x), y(y), txt(txt), val(val), flag(flag)
{
    w = 10 + gui_strlen(txt);
    h = 6 + text_height(font);
}

int BUTTON::is_over(int ox, int oy, bool already_pressed)
{
    if (ox > x && oy > y && ox < x + w && oy < y + h) {
        if (!already_pressed && (mouse_b & 1)) *flag = val;
        return true;
    }
    return false;
}

void BUTTON::draw(BITMAP* buffer)
{
    rect(buffer, x, y, x + w, y + h, makecol(43,79,112));
    rectfill(buffer, x + 1, y + 1, x + w - 1, y + h - 1, is_over(mouse_x, mouse_y) ? makecol(230,230,230) : makecol(175,175,175));
    gui_textout_ex(buffer, txt, x + w/2, y + 3, makecol(0,0,0), -1, 1);
}

void maps_filllist(void)
{
    struct al_ffblk info;
    string tmp;

    maps.clear();

    if (al_findfirst("data/maps/*.map", &info, FA_ALL)) return;

    do {
        tmp = info.name;
        tmp.erase(tmp.rfind(".map"));
        maps.push_back(tmp);
    } while (!al_findnext(&info));

    al_findclose(&info);
}

// ----------------  MESSY DIALOG STUFF ----------------

int slider_callback(void * dp3, int d2)
{
    static char buf[64];
    static int size;
    DIALOG* d = (DIALOG*)dp3;

    if (!d || !d->dp) return D_O_K;

    size = (d->d1 - d->d2) * d2 / 1000 + d->d2;
    printf("%d\n",size);
    snprintf(buf, 64, "%d", size);
    d->dp = buf;
    d->dp3 = (void*)&size;

    broadcast_dialog_message(MSG_DRAW, 0);

    return D_O_K;
}

char * listgamesizes_callback(int index, int * size)
{
    static char buffer[20];
    if (index < 0) {
        *size = 7;
        return NULL;
    }
    sprintf(buffer, "  %i", index + 2);
    return buffer;
}

char * listtiles_callback(int index, int * size)
{
    static char buffer[128];
    if (index < 0) {
        *size = etool->tiles.size();
        return NULL;
    }
    snprintf(buffer, 128, " %s", etool->tiles[index].c_str());
    return buffer;
}

char * listobjs_callback(int index, int * size)
{
    static char buffer[128];
    if (index < 0) {
        *size = etool->stationaries.size();
        return NULL;
    }
    snprintf(buffer, 128, " %s", etool->stationaries[index].c_str());
    return buffer;
}

char * listplayers_callback(int index, int * size)
{
    static char buffer[20];
    if (index < 0) {
        *size = etool->players;
        return NULL;
    }
    sprintf(buffer, "  %i", index + 1);
    return buffer;
}

char * listunits_callback(int index, int * size)
{
    static char buffer[128];
    if (index < 0) {
        *size = etool->units.size();
        return NULL;
    }
    snprintf(buffer, 128, " %s", etool->units[index].c_str());
    return buffer;
}

char * listbuildings_callback(int index, int * size)
{
    static char buffer[128];
    if (index < 0) {
        *size = etool->buildings.size();
        return NULL;
    }
    snprintf(buffer, 128, " %s", etool->buildings[index].c_str());
    return buffer;
}

char * listplayerciv_callback(int index, int * size)
{
    static char buffer[20];
    if (index < 0) {
        *size = etool->players;
        return NULL;
    }
    sprintf(buffer, "  %i - %s", index + 1, game_obj->get_plcivname(index + 1));
    return buffer;
}

char * listcivs_callback(int index, int * size)
{
    static char buffer[20];
    if (index < 0) {
        *size = etool->civs.size();
        return NULL;
    }
    sprintf(buffer, " %s", etool->civs[index].c_str());
    return buffer;
}

const char * listmaps_callback(int index, int * size)
{
    if (index < 0) {
        *size = maps.size();
        return NULL;
    }
    return maps[index].c_str();
}

DIALOG DIALOG_INTROMENU[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { lex_dialog_proc,   0,      0,    440,  220,   -1,  -1,    0,    0,         1,                      0,    NULL,    NULL, NULL  },
   { lex_button_proc,   97,    60,   100,   24,    0,  -1,    'h',  D_EXIT,    -1,                     1,    (void*)"&Host Game",     NULL, NULL  },
   { lex_button_proc,   243,    60,   100,   24,    0,  -1,    'j',  D_EXIT,    -1,                     2,    (void*)"&Join Game",     NULL, NULL  },
   { lex_button_proc,   170,    120,   100,   24,    0,  -1,    'e',  D_EXIT,    -1,                     3,    (void*)"Map &Editor",     NULL, NULL  },
   { lex_button_proc,   170,    170,   100,   24,    0,  -1,    'x',  D_EXIT,    -1,                     0,    (void*)"E&xit",     NULL, NULL  },
   { NULL,              0,      0,    0,    0,    0  ,   0,    0,    0,          0,                     0,    NULL,         NULL, NULL  },
};

DIALOG DIALOG_MAPEDITOR[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { lex_dialog_proc,   0,      0,    500,  300,   -1,  -1,    0,    0,         1,                      0,    (void*)"Map Editor",    NULL, NULL  },

   // map size
   { d_text_proc,    190,  100,   100 ,  15,    0,  -1,    0,    0,    MAX_MAP_SIZE,          MIN_MAP_SIZE,    (void*)"",   NULL, NULL  },
   { d_rtext_proc,    100,  100,   90 ,  15,    0,  -1,    0,    0,    0,                               0,      (void*)"Map size: ",        NULL, NULL  },
   { lex_slider_proc,   60,  130,  200,   20,   0 , -1,    0,    0,           1000,                      0,    NULL,        (void*)slider_callback, (void*)&DIALOG_MAPEDITOR[1]  },

   // number of players
   { d_text_proc,    360,  65,   100 ,  15,    0,  -1,    0,    0,         0,                         0,    (void*)"Players",   NULL, NULL  },
   { lex_list_proc ,    360,  90,  50 ,  140,    0  ,-1,    0,    0,          0,                      0,   (void*)listgamesizes_callback,        NULL, NULL  },

    // buttons
   { lex_button_proc,   20,    264,   70,   24,    0,  -1,    'b',  D_EXIT,    -1,                     0,    (void*)"&Back",     NULL, NULL  },
   { lex_button_proc,   280,    264,   100,   24,    0,  -1,    'l',  D_EXIT,    -1,                     3,    (void*)"&Load Map",     NULL, NULL  },
   { lex_button_proc,   410,    264,   70,   24,    0,  -1,    'c',  D_EXIT,    -1,                     2,    (void*)"&Create",     NULL, NULL  },
   { NULL,              0,      0,    0,    0,    0  ,   0,    0,    0,          0,                     0,    NULL,         NULL, NULL  },
};

DIALOG DIALOG_TILEEDITOR[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { lex_dialog_proc,   0,      0,    500,  300,   -1,  -1,    0,    0,         0,                      0,    (void*)"Tile Brush Optons",    NULL, NULL  },

   // tile size
   { d_text_proc,    375,  100,   100 ,  15,    0,  -1,    0,    0,    8,          1,    (void*)"",   NULL, NULL  },
   { d_rtext_proc,    285,  100,   90 ,  15,    0,  -1,    0,    0,    0,                               0,      (void*)"Brush size: ",        NULL, NULL  },
   { lex_slider_proc,   270,  130,  150,   20,   0 , -1,    0,    0,           1000,                      0,    NULL,        (void*)slider_callback, (void*)&DIALOG_TILEEDITOR[1]  },

   // tile types
   { d_text_proc,    75,  65,   100 ,  15,    0,  -1,    0,    0,         0,                         0,    (void*)"Tile Type",   NULL, NULL  },
   { lex_list_proc ,    75,  90,  120 ,  140,    0  ,-1,    0,    0,          0,                      0,   (void*)listtiles_callback,        NULL, NULL  },
   { lex_check_proc ,    280,  200,  120 ,  20,    0  ,-1,    0,    D_SELECTED,          1,           1,   (void*)"Erase objects",        NULL, NULL  },
   { lex_check_proc ,    280,  180,  120 ,  20,    0  ,-1,    0,    0,          1,                      1,   (void*)"Rotate brush",        NULL, NULL  },

    // buttons
    { lex_button_proc,   80,    264,   100,   24,    0,  -1,    'c',  D_EXIT,    0,                     3,    (void*)"&Clear Map",     NULL, NULL  },
   { lex_button_proc,   410,    264,   70,   24,    0,  -1,    'o',  D_EXIT,    -1,                     2,    (void*)"&Ok",     NULL, NULL  },
   { NULL,              0,      0,    0,    0,    0  ,   0,    0,    0,          0,                     0,    NULL,         NULL, NULL  },
};

DIALOG DIALOG_OBJEDITOR[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { lex_dialog_proc,   0,      0,    500,  300,   -1,  -1,    0,    0,         0,                      0,    (void*)"Object Clump Generator",    NULL, NULL  },

   // tile size
   { d_text_proc,    375,  100,   100 ,  15,    0,  -1,    0,    0,    50,          1,    (void*)"",   NULL, NULL  },
   { d_rtext_proc,    285,  100,   90 ,  15,    0,  -1,    0,    0,    0,                               0,      (void*)"Seed Size: ",        NULL, NULL  },
   { lex_slider_proc,   245,  130,  200,   20,   0 , -1,    0,    0,           1000,                      100,    NULL,        (void*)slider_callback, (void*)&DIALOG_OBJEDITOR[1]  },

   // tile types
   { d_text_proc,    75,  65,   100 ,  15,    0,  -1,    0,    0,         0,                         0,    (void*)"Object Type",   NULL, NULL  },
   { lex_list_proc ,    75,  90,  120 ,  140,    0  ,-1,    0,    0,          0,                      0,   (void*)listobjs_callback,        NULL, NULL  },
   { lex_check_proc ,    280,  200,  120 ,  20,    0  ,-1,    0,    D_SELECTED,          1,           0,   (void*)"Enable branching",        NULL, NULL  },

    // buttons
   { lex_button_proc,   410,    264,   70,   24,    0,  -1,    'o',  D_EXIT,    -1,                     2,    (void*)"&Ok",     NULL, NULL  },
   { NULL,              0,      0,    0,    0,    0  ,   0,    0,    0,          0,                     0,    NULL,         NULL, NULL  },
};

DIALOG DIALOG_BUILDINGEDITOR[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { lex_dialog_proc,   0,      0,    500,  300,   -1,  -1,    0,    0,         0,                      0,    (void*)"Building and Build Site Placement",    NULL, NULL  },

   // building types
   { d_text_proc,    230,  45,   200 ,  15,    0,  -1,    0,    0,         0,                         0,    (void*)"Building / Build Site",   NULL, NULL  },
   { lex_list_proc ,    230,  70,  225 ,  160,    0  ,-1,    0,    0,          0,                      0,   (void*)listbuildings_callback,        NULL, NULL  },

   // player
   { d_text_proc,    75,  45,   100 ,  15,    0,  -1,    0,    0,         0,                         0,    (void*)"Player*",   NULL, NULL  },
   { lex_list_proc ,    75,  70,  50 ,  160,    0  ,-1,    0,    0,          0,                      0,   (void*)listplayers_callback,        NULL, NULL  },
   { d_text_proc,    30,  248,   200 ,  12,    0,  -1,    0,    0,         0,                         0,    (void*)"*the Player choice is ignored",   NULL, NULL  },
   { d_text_proc,    30,  265,   200 ,  12,    0,  -1,    0,    0,         0,                         0,    (void*)" for generic build sites",   NULL, NULL  },

    // buttons
   { lex_button_proc,   410,    264,   70,   24,    0,  -1,    'o',  D_EXIT,    -1,                     2,    (void*)"&Ok",     NULL, NULL  },
   { NULL,              0,      0,    0,    0,    0  ,   0,    0,    0,          0,                     0,    NULL,         NULL, NULL  },
};

DIALOG DIALOG_UNITEDITOR[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { lex_dialog_proc,   0,      0,    500,  350,   -1,  -1,    0,    0,         0,                      0,    (void*)"Unit Placement",    NULL, NULL  },

   // building types
   { d_text_proc,    230,  45,   200 ,  15,    0,  -1,    0,    0,         0,                         0,    (void*)"Unit Type",   NULL, NULL  },
   { lex_list_proc ,    230,  70,  225 ,  160,    0  ,-1,    0,    0,          0,                      0,   (void*)listunits_callback,        NULL, NULL  },

   // player
   { d_text_proc,    75,  45,   100 ,  15,    0,  -1,    0,    0,         0,                         0,    (void*)"Player",   NULL, NULL  },
   { lex_list_proc ,    75,  70,  50 ,  160,    0  ,-1,    0,    0,          0,                      0,   (void*)listplayers_callback,        NULL, NULL  },

   // group size
   { d_text_proc,    250,  280,   100 ,  15,    0,  -1,    0,    0,    GROUP_MAXUNITS,          1,    (void*)"",   NULL, NULL  },
   { d_rtext_proc,    150,  280,   100 ,  15,    0,  -1,    0,    0,    0,                               0,      (void*)"Group Size: ",        NULL, NULL  },
   { lex_slider_proc,   140,  300,  150,   20,   0 , -1,    0,    0,           1000,                      0,    NULL,        (void*)slider_callback, (void*)&DIALOG_UNITEDITOR[5] },
   
   // buttons
   { lex_button_proc,   410,    314,   70,   24,    0,  -1,    'o',  D_EXIT,    -1,                     2,    (void*)"&Ok",     NULL, NULL  },
   { NULL,              0,      0,    0,    0,    0  ,   0,    0,    0,          0,                     0,    NULL,         NULL, NULL  },
};

DIALOG DIALOG_PLAYEREDITOR[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { lex_dialog_proc,   0,      0,    400,  370,   -1,  -1,    0,    0,         1,                      0,    (void*)"Player Settings",    NULL, NULL  },

   // player
   { d_text_proc,    75,  45,   100 ,  15,    0,  -1,    0,    0,         0,                         0,    (void*)"Player",   NULL, NULL  },
   { lex_list_proc ,    75,  70,  200 ,  160,    0  ,-1,    0,    0,          0,                      0,   (void*)listplayerciv_callback,        NULL, NULL  },

    // buttons
   { lex_button_proc,   75,    254,   175,   24,    0,  -1,    'c',  D_EXIT,    0,                     1,    (void*)"&Change Civilization",     NULL, NULL  },
   { lex_button_proc,   310,    334,   70,   24,    0,  -1,    'd',  D_EXIT,    -1,                     0,    (void*)"&Done",     NULL, NULL  },
   { lex_button_proc,   20,    334,   100,   24,    0,  -1,    'd',  D_EXIT,    -1,                     2,    (void*)"Set Resources",     NULL, NULL  },
   { NULL,              0,      0,    0,    0,    0  ,   0,    0,    0,          0,                     0,    NULL,         NULL, NULL  },
};

DIALOG DIALOG_CIVEDITOR[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { lex_dialog_proc,   0,      0,    300,  280,   -1,  -1,    0,    0,         1,                      0,    (void*)"Select Player Civilization",    NULL, NULL  },

   // player
   { d_text_proc,    75,  45,   100 ,  15,    0,  -1,    0,    0,         0,                         0,    (void*)"Civilizations",   NULL, NULL  },
   { lex_list_proc ,    75,  70,  150 ,  160,    0  ,-1,    0,    0,          0,                      0,   (void*)listcivs_callback,        NULL, NULL  },

    // buttons
   { lex_button_proc,   200,    244,   70,   24,    0,  -1,    's',  D_EXIT,    -1,                     0,    (void*)"&Set",     NULL, NULL  },
   { NULL,              0,      0,    0,    0,    0  ,   0,    0,    0,          0,                     0,    NULL,         NULL, NULL  },
};

DIALOG DIALOG_SAVE[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { lex_dialog_proc,   0,  0,  440,  150,   -1,  -1,    0,    0,    0,        0,                           (void*)"Save Map",    NULL, NULL  },
   { d_rtext_proc,    40,  60,   160 ,  120,    0,  -1,    0,    0,    1,        0,                          (void*)"Map Name: ",        NULL, NULL  },
   { lex_edit_proc,    200,  60,   200 ,  24,    0,  -1,    0,    0,    256,        0,                          NULL,        NULL, NULL  },
   { lex_button_proc,   10,  114,   50,   24,    0,  -1,    'c',  D_EXIT,    -1,                     0,    (void*)"&Cancel",     NULL, NULL  },
   { lex_button_proc,   380,  114,   50,   24,    0,  -1,    'a',  D_EXIT,    -1,                     1,    (void*)"S&ave",     NULL, NULL  },
   { NULL,              0,    0,    0,    0,    0  ,   0,    0,    0,          0,                     0,    NULL,         NULL, NULL  },
};

DIALOG DIALOG_TEXTIN[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { lex_dialog_proc,   0,  0,  220,  150,   -1,  -1,    0,    0,    0,        0,                           (void*)"title",    NULL, NULL  },
   { lex_edit_proc,    10,  60,   200 ,  24,    0,  -1,    0,    0,    256,        0,                          NULL,        NULL, NULL  },
   { lex_button_proc,   10,  114,   50,   24,    0,  -1,    'c',  D_EXIT,    -1,                     0,    (void*)"&Cancel",     NULL, NULL  },
   { lex_button_proc,   160,  114,   50,   24,    0,  -1,    'a',  D_EXIT,    -1,                     1,    (void*)"&Ok",     NULL, NULL  },
   { NULL,              0,    0,    0,    0,    0  ,   0,    0,    0,          0,                     0,    NULL,         NULL, NULL  },
};

DIALOG DIALOG_LOAD[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { lex_dialog_proc,   0,  0,  300,  350,   -1,  -1,    0,    0,    0,        0,                           (void*)"Choose Map",    NULL, NULL  },
   { lex_list_proc ,    50,  70,  200 ,  200,    0  ,-1,    0,    0,          0,                      0,   (void*)listmaps_callback,        NULL, NULL  },
   { d_ctext_proc,    100,  50,   100 ,  15,    0,  -1,    0,    0,         0,                         0,    (void*)"Map",   NULL, NULL  },
   { lex_button_proc,   10,  314,   50,   24,    0,  -1,    'c',  D_EXIT,    -1,                     0,    (void*)"&Cancel",     NULL, NULL  },
   { lex_button_proc,   240,  314,   50,   24,    0,  -1,    'l',  D_EXIT,    -1,                     1,    (void*)"&Load",     NULL, NULL  },
   { NULL,              0,    0,    0,    0,    0  ,   0,    0,    0,          0,                     0,    NULL,         NULL, NULL  },
};

DIALOG DIALOG_MESSAGE[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { lex_dialog_proc,   0,  0,  440,  150,   -1,  -1,    0,    0,    0,        0,                           NULL,    NULL, NULL  },
   { d_ctext_proc,    20,  60,   400 ,  120,    0,  -1,    0,    0,    1,        0,                          NULL,        NULL, NULL  },
   { lex_button_proc,   195,  114,   50,   24,    0,  -1,    'o',  D_EXIT,    -1,                     0,    (void*)"&Ok",     NULL, NULL  },
   { NULL,              0,    0,    0,    0,    0  ,   0,    0,    0,          0,                     0,    NULL,         NULL, NULL  },
};

void GUI::map_editor(void)
{
    int r;
    r = run_dialog(DIALOG_MAPEDITOR, true);

    if (r >= 0 && DIALOG_MAPEDITOR[r].d2 > 1)
        new_gameedit(*(int*)DIALOG_MAPEDITOR[1].dp3, DIALOG_MAPEDITOR[5].d1 + 2, (DIALOG_MAPEDITOR[r].d2 == 3));
}

void GUI::tile_editor(void)
{
    int r;

    r = run_dialog(DIALOG_TILEEDITOR, true);

    etool->brush_size = *(int*)DIALOG_TILEEDITOR[1].dp3;
    etool->cur_tile = DIALOG_TILEEDITOR[5].d1;
    etool->cur_tool_options = (DIALOG_TILEEDITOR[6].flags & D_SELECTED);
    etool->brush_shape = (DIALOG_TILEEDITOR[7].flags & D_SELECTED);
    etool->cur_tool = EDITOR_TOOL::TOOL_TILE;

    // fill
    if (r >= 0 && DIALOG_TILEEDITOR[r].d2 == 3)
        game->tool_tilefill();
}

void GUI::object_editor(void)
{
    int r;

    r = run_dialog(DIALOG_OBJEDITOR, true);

    etool->brush_size = *(int*)DIALOG_OBJEDITOR[1].dp3;
    etool->cur_stationary = DIALOG_OBJEDITOR[5].d1;
    etool->cur_tool_options = (DIALOG_OBJEDITOR[6].flags & D_SELECTED);
    etool->cur_tool = EDITOR_TOOL::TOOL_OBJ;
}

void GUI::unit_editor(void)
{
    int r;

    r = run_dialog(DIALOG_UNITEDITOR, true);

    etool->brush_size = *(int*)DIALOG_UNITEDITOR[5].dp3;
    etool->pl = DIALOG_UNITEDITOR[4].d1 + 1;
    etool->cur_building = DIALOG_UNITEDITOR[2].d1;
    etool->cur_tool = EDITOR_TOOL::TOOL_PLACE;
}

void GUI::building_editor(void)
{
    int r;

    r = run_dialog(DIALOG_BUILDINGEDITOR, true);

    etool->pl = DIALOG_BUILDINGEDITOR[4].d1 + 1;
    etool->cur_building = DIALOG_BUILDINGEDITOR[2].d1;
    etool->cur_tool = EDITOR_TOOL::TOOL_BUILD;
}

void GUI::civ_editor(void)
{
    int r, r2, pl;

    do {
        // select player
        r = run_dialog(DIALOG_PLAYEREDITOR, true);
        if (r >= 0) r = DIALOG_PLAYEREDITOR[r].d2;

        pl = DIALOG_PLAYEREDITOR[2].d1 + 1;

        // set civ dialog
        if (r == 1) {
            DIALOG_CIVEDITOR[2].d1 = game_obj->get_plciv(pl);
            r2 = run_dialog(DIALOG_CIVEDITOR, true);
            if (r2 >= 0) game_obj->set_plciv(pl, DIALOG_CIVEDITOR[2].d1);
        // set starting resources dialog
        } else if (r == 2) {
            resources_dialog();
        }

    } while (r > 0);
}

void GUI::save_dialog(void)
{
    int r;
    static char buf[256];

    DIALOG_SAVE[2].d2 = strlen(etool->name);
    DIALOG_SAVE[2].dp = (void*)etool->name;

    r = run_dialog(DIALOG_SAVE, true, 2);

    // save map
    if (r >= 0 && DIALOG_SAVE[r].d2 == 1 && strlen(etool->name)) {
        string fname = string("data/maps/") + string(etool->name) + string(".map");
        if (game->save_map(fname.c_str()))
            display_message("", "Map Saved!");
        else
            display_message("Error", "Problem saving map.");
    }

    DIALOG_SAVE[2].dp = (void*)buf;
}

void GUI::resources_dialog(void)
{
    char buf[1024];
    int val;

    if (!game) return;

    snprintf(buf, 1024, "%d", game->get_startingresources());

    std::string ret = text_dialog("Starting Resources", buf);

    if (!ret.length()) return;

    val = atoi(ret.c_str());
    if (val >= 0) game->set_startingresources(val);
}

std::string GUI::text_dialog(const char * title, const char * start_val)
{
    int r;
    char buf[1024];

    if (start_val) strcpy(buf, start_val);
    else buf[0] = '\0';

    DIALOG_TEXTIN[0].dp = (void*)title;
    DIALOG_TEXTIN[1].d2 = 0;
    DIALOG_TEXTIN[1].dp = (void*)buf;

    r = run_dialog(DIALOG_TEXTIN, true, 1);

    // return string
    if (r >= 0 && DIALOG_TEXTIN[r].d2 == 1) {
        return string(buf);
    }

    return string("");
}

const char * GUI::load_dialog(void)
{
    int r;
    const char * rbuf;

    maps_filllist();

    if (!maps.size()) {
        message("","No maps found.");
        return NULL;
    }

    r = run_dialog(DIALOG_LOAD, true, 2);

    rbuf = maps[DIALOG_LOAD[1].d1].c_str();

    // save map
    if (r >= 0 && DIALOG_LOAD[r].d2 == 1 && strlen(rbuf)) {
        return rbuf;
    }

    return NULL;
}

void GUI::message(const char * title, const char * msg)
{
    if (!game || !game->is_loaded()) {
        DIALOG_MESSAGE[0].dp = (void*)title;
        DIALOG_MESSAGE[1].dp = (void*)msg;
        run_dialog(DIALOG_MESSAGE, true);
    } else {
        string str = strlen(title) ? string(title) + ": " + string(msg) : string(msg);
        game->message(str);
    }
}

void GUI::display_message(const char * title, const char * msg)
{
    DIALOG_MESSAGE[0].dp = (void*)title;
    DIALOG_MESSAGE[1].dp = (void*)msg;
    display_dialog(DIALOG_MESSAGE, true);
}

void draw_screenobj(BITMAP* buffer)
{
    blit(buffer, screen_obj->get_page(), 0, 0, 0, 0, screen_obj->get_width(), screen_obj->get_height());
    draw_mouse(screen_obj->get_page(), mouse_cursor);
    screen_obj->flip_page();
}

void draw_mouse(BITMAP* buffer, BITMAP* mouse)
{
    masked_blit(mouse, buffer, 0, 0, mouse_x + 1, mouse_y + 1, mouse->w, mouse->h);
}
