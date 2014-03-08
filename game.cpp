#include "wotwar.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>

extern GLOBAL_CONFIG global_config;

// just temporary (Yea.. right. I know how that works out.)
#define DRAW_LBAR(p) {clear_to_color(buffer, makecol(0,0,0)); draw_loadbar(buffer, vscreen.get_width()/2, vscreen.get_height()/2, vscreen.get_width()/2, 25, makecol(255,255,255), makecol(200,200,200), (p)); vscreen.flip_page();}

GAME::GAME(BITMAP*& buffer, SCREEN& vscreen, UI_DATA& ui_data, GUI& gui, int num_players, int player_id, const char * map_file, int size_map, bool edit_mode, bool is_host, bool enable_networking, const char * hostname) : buffer(buffer), gui(gui), vscreen(vscreen), active_player(player_id), host_name(hostname), background_mode(!enable_networking && !edit_mode), is_host(is_host), starting_resources(500), game_won_by(0), edit_mode(edit_mode), mtimer(0), click_time(0), network(NULL), is_fight(false), game_loaded(false), esc_mode(false), curTileX(-1), curTileY(-1), tmp_grp(NULL), failed_load(false), mouse_l(false), mouse_r(false), alreadyPressedL(false), alreadyPressedR(false)
{
    FILE * fin = NULL;
    int tmp;
    string fname("");

    // join game
    if (!edit_mode && !is_host && enable_networking) {
        // connect to host
        draw_ltext("client: connecting to host");
        network = new NETWORK(global_config.NETWORK_PORT);
        if (!network->connect_player(host_name)) {
            gui.message("Error", "Unable to connect to host.");
            failed_load = true;
        }
        // get map
        draw_ltext("transferring map data");
        string tfname = network->get(0)->rcv_file();
        map_file = tfname.c_str();
        // get id
        draw_ltext("waiting for players to join");
        network->get(0)->rcv(player_id);
        active_player = player_id;
        network->listen_messages(is_host);
        if (!network->get(0)->is_connected()) failed_load = true;
    }

    // check if load map
    if (!map_file || failed_load) etool.name[0] = '\0';
    else {
        strncpy(etool.name, map_file, sizeof(etool.name));

        // start loading map
        if (is_host || edit_mode) fname = string("data/maps/") + string(map_file) + string(".map");
        else fname = string(map_file);

        fin = open_header(fname.c_str());
    }

    // read map size
    tmp = size_map;
    if (fin && !failed_load) fscanf(fin, "%d %d", &size_map, &tmp);

    // read starting resources
    if (fin && !failed_load) {
        fscanf(fin, "%d", &starting_resources);
    }

    // read number of players
    if (fin && !failed_load) {
        fscanf(fin, "%d", &num_players);
    }

	// check player bounds
	if (num_players > 8) num_players = 8;
	if (num_players < 1) num_players = 1;
	if (background_mode) num_players = 2;
	if (player_id > num_players)
        panic("player_id larger than num_players!");

	// set player stuff
	this->num_players = num_players;
	etool.players = num_players;
	if (edit_mode) active_player = 0;

	// host game
    if (!edit_mode && is_host && enable_networking) {
        player_id = 1;
        draw_ltext("host: creating sockets");
        network = new NETWORK(global_config.NETWORK_PORT);
        if (!network->any_error()) {
            // send maps
            host_tx_file(fname);
            if (!failed_load) {
                // send player_ids
                draw_ltext("All players connected. Syncing.");
                network->listen_messages(is_host);
                char id=2;
                for (int i=0; i<num_players-1; i++) {
                    network->get(i)->snd(id++);
                }
            }
        } else {
            failed_load = true;
        }
    }

	// Loading text
	clear_to_color(buffer, makecol(0,0,0));

	DRAW_LBAR(0);

	// setup
	init_players();

	if (fin && !failed_load) {
	    load_players(fin);
	    load_check(fin);
	}

	if (size_map < 25 || tmp < 25) panic("map size is too small");
	world = new WORLD(size_map, tmp);

    // load map
	if (fin && !failed_load) {
	    world->load_tiles(fin);
	    load_check(fin);
	    world->load_stationaries(fin);
	}

    // create all game objects
	DRAW_LBAR(30);

	display = new DISPLAY(buffer, vscreen.get_extrapage(), world, &player[active_player], active_player, 0, 0, vscreen.get_width(), vscreen.get_height());

	DRAW_LBAR(50);

	data = new DATA("data/", world, ui_data);

	DRAW_LBAR(65);

	attack_data = new ATTACK_DATA("data/");

	DRAW_LBAR(75);

	particle_sys = new PARTICLE_SYSTEM("data/", this);

	DRAW_LBAR(90);

	agentHandler = new AGENT_HANDLER(network, display, world->getPather(), data, *particle_sys, *attack_data, player);

	// load buildings
	if (fin && !failed_load) {
        agentHandler->load_agents(fin);
	}

	printf("done loading.\n\n");

	DRAW_LBAR(100);

	// close file
	if (fin) fclose(fin);

	// background mode
	if (background_mode) {
	    // randomize player factions
	    for (int i=1; i<player.size(); i++)
            player[i].set_faction(rand()%data->get_numbcivs());

        // center display
        display->worldCenterOver(world->getWidth()/2, world->getHeight()/2);
	}

	// init
	alreadyPressedL = false;
	alreadyPressedR = false;
	overlayClickL = false;
	overlayClickR = false;
	setup_editor();

	// start over a building
	if (active_player) {
        GROUP* ag = agentHandler->get_building(active_player);
        if (ag) display->worldCenterOver(ag->get_x(), ag->get_y());
	}

	// sync
	if (!edit_mode && !failed_load && enable_networking) {
	    draw_ltext("waiting for all players to load");
        if (!is_host) network->send_all_sync();
        if (!network->wait_all_sync()) {
            failed_load = true;
            gui.message("Error", "Connection closed.");
        }
        if (is_host) network->send_all_sync();
	    display->addMessage("Press <ENTER> to send a message to all players.");
	    display->addMessage("Press <ESC> to leave the game.");
	}

	game_loaded = true;

	clear_speedcounter();
}

GAME::~GAME(void) {
	puts("destructing core objects:\n~game");
	if (network) {
	    puts(" ~network");
        delete network;
	}
	puts(" ~agent_handler");
	delete agentHandler;
    puts(" ~particle_sys");
	delete particle_sys;
	puts(" ~attack_data");
	delete attack_data;
	puts(" ~data");
	delete data;
	puts(" ~display");
	delete display;
	puts(" ~world");
	delete world;
	puts("done.");
}

bool GAME::host_tx_file(string mapname)
{
    char buf[256];
    int sent_map = 0;
    int connected = 0;
    int tmp;
    if (!network->accept_connections()) {
        failed_load = true;
        return false;
    }
    string my_ip = string("hostname: ") + network->hostname();
    draw_ltext("host: waiting for players to join", my_ip.c_str());
    while (sent_map < num_players-1 && !failed_load) {
        // fail on user request
        if (key[KEY_ESC]) {
            failed_load = true;
            return false;
        }

        // connect to all players
        if (network->num_connected() > sent_map) {
            // wait for next player
            snprintf(buf, 256, "player %d of %d connected, sending map...", sent_map+2, num_players);
            draw_ltext(buf, my_ip.c_str());
            // send map file
            network->get(sent_map++)->send_file(mapname);

            // waiting text
            if (sent_map < num_players-1) {
                snprintf(buf, 256, "waiting for player %d of %d to join", sent_map+2, num_players);
                draw_ltext(buf, my_ip.c_str());
            }
        }
    }

    return true;
}

void GAME::draw_ltext(const char * str, const char * str2)
{
    clear_to_color(buffer, makecol(0,0,0));
    textout_centre_ex(buffer, font, str, buffer->w/2, buffer->h/2, makecol(200,200,200), -1);
    textout_centre_ex(buffer, font, str2, buffer->w/2, buffer->h/2 + 60, makecol(200,200,200), -1);
    vscreen.flip_page();
}

bool GAME::is_failed(void)
{
    return failed_load;
}

void GAME::end_game(void)
{
    failed_load = true;
    if (!network) return;
    char str[1024];
    snprintf(str, 1024, "Player %d (%s) has left the game.", active_player, get_plcivname(active_player));
    sendMessage(str);
}

FILE * GAME::open_header(const char * fname)
{
    FILE * fin;
    char buf[128];

    // start loading map
    fin = fopen(fname, "r");
    if (!fin) {
        perror("fopen()");
        gui.message("Error", "Unable to open map file.");
        failed_load = true;
        return NULL;
    }

    fgets(buf, 128, fin);

    // check if a map file
    if (strncmp(buf, "MapFile", 7)) {
        gui.message("Error", "Unable to open map file.");
        failed_load = true;
        return NULL;
    }

    return fin;
}

void GAME::load_players(FILE * fin)
{
    int tmp;

    for (int i=1; i<=num_players; i++) {
        fscanf(fin, "%d", &tmp);
        set_plciv(i, tmp);
    }
}

bool GAME::load_check(FILE * fin)
{
    char buf[32];
    do {
        fgets(buf, 32, fin);
    } while (strlen(buf) < 3);

    if (strncmp(buf, "end", 3)) {
        gui.message("Error", "Corrupt map file.");
        failed_load = true;
        return false;
    }
    return true;
}

EDITOR_TOOL::EDITOR_TOOL(void) : players(0), brush_size(1), cur_tile(0), cur_stationary(0), cur_building(0), cur_tool_options(0)
{
}

void EDITOR_TOOL::draw_tools(DISPLAY * display, int worldx, int worldy)
{
    int dx, dy, os;

    switch (cur_tool) {
        case EDITOR_TOOL::TOOL_TILE :
            if (!brush_shape) {
                dx = worldx / TILE_SIZE - brush_size/2;
                dy = worldy / TILE_SIZE - brush_size/2;
                rect(display->buffer, display->toScreenX(dx*TILE_SIZE), display->toScreenY(dy*TILE_SIZE), display->toScreenX((dx + brush_size) * TILE_SIZE), display->toScreenY((dy + brush_size) * TILE_SIZE), makecol(255,255,255));
            } else {
                dx = display->toScreenX((worldx / TILE_SIZE - (brush_size%2 ? 0 : 1))*TILE_SIZE);
                dy = display->toScreenY((worldy / TILE_SIZE - brush_size + 1)*TILE_SIZE);
                os = (brush_size*2 - 1) * TILE_SIZE;
                line(display->buffer, dx + TILE_SIZE/2, dy - TILE_SIZE/2, dx + os/2 + TILE_SIZE, dy + os/2, makecol(255,255,255));
                line(display->buffer, dx + TILE_SIZE/2, dy - TILE_SIZE/2, dx - os/2, dy + os/2, makecol(255,255,255));
                line(display->buffer, dx + os/2 + TILE_SIZE, dy + os/2, dx + TILE_SIZE/2, dy + os + TILE_SIZE/2, makecol(255,255,255));
                line(display->buffer, dx - os/2, dy + os/2, dx + TILE_SIZE/2, dy + os + TILE_SIZE/2, makecol(255,255,255));
            }
        break;

        case EDITOR_TOOL::TOOL_OBJ :
            dx = display->toScreenX(worldx);
            dy = display->toScreenY(worldy);
            line(display->buffer, dx - 25, dy - 40, dx + 25, dy - 40, makecol(255,255,255));
            line(display->buffer, dx - 25, dy + 40, dx + 25, dy + 40, makecol(255,255,255));
            line(display->buffer, dx + 40, dy - 25, dx + 40, dy + 25, makecol(255,255,255));
            line(display->buffer, dx - 40, dy - 25, dx - 40, dy + 25, makecol(255,255,255));
        break;

        case EDITOR_TOOL::TOOL_BUILD :
            dx = ((int)worldx/SMALL_AREA_SIZE)*SMALL_AREA_SIZE + SMALL_AREA_SIZE/2;
            dy = ((int)worldy/SMALL_AREA_SIZE)*SMALL_AREA_SIZE + SMALL_AREA_SIZE/2;
            circle(display->buffer, display->toScreenX(dx), display->toScreenY(dy), SMALL_AREA_SIZE/2, makecol(255,255,255));
        break;
    }
}

inline bool distIsLessThan(int toX, int toY, int distance) {
	if (toX * toX + toY * toY <= distance * distance) return true;
	else return false;
}

EDITOR_TOOL * GAME::get_etool(void)
{
    return &etool;
}

bool GAME::is_editmode(void)
{
    return edit_mode;
}

void GAME::setup_editor(void)
{
    etool.players = num_players;
    etool.tiles = world->tile_types;
    etool.stationaries = world->stationary_types;
    etool.buildings = data->building_list;
	etool.units = data->unit_list;
    etool.civs = data->civs;
}

void GAME::sendMessage(char *str)
{
    MESSAGE msg;
    msg.type = MSGTYPE_CHAT;
    msg.size = sizeof(msg);
    msg.player_id = get_player();
    msg.msg.chat.len = strlen(str)+1;
    msg.msg.chat.str = str;

    network->broadcast((char*)&msg, sizeof(msg));
    network->broadcast(msg.msg.chat.str, msg.msg.chat.len);
}

// the game
void GAME::update(bool draw)
{
    // check for network messages
    if (network) update_network();
	// get mouse input
	if (!background_mode) mouseInput();
	// draw background
	if (draw) display->drawWorld();
	// draw and update below particles
	particle_sys->update_and_draw(*display, draw, false);
	// draw and update agents
	agentHandler->updateAll(draw);
	// draw and update above particles
	particle_sys->update_and_draw(*display, draw, true);
	// draw trees, etc.
	if (draw) display->drawOverhead();
	// draw task-to-direction arrow
	if (mouse_r && draw && !overlayClickR && !edit_mode && !distIsLessThan(x1 - curX, y1 - curY, DRAG_DIST_TO_ARROW)) display->drawArrow(x1, y1, (int) (atan2(curY - y1, curX - x1) / (2 * M_PI) * 256.0)); // draw direction arrow
	// draw selection box
	if (mouse_l && draw && !overlayClickL && !edit_mode && !distIsLessThan(x1 - curX, y1 - curY, 25)) display->drawSelectBox(display->toScreenX(x1), display->toScreenY(y1), mouse_x, mouse_y);
	// draw editor tools
	if (edit_mode && draw) etool.draw_tools(display, curX, curY);
	// check if someone has won the game
	else if (is_host && !game_won_by && agentHandler->check_winner()) {
	    game_won_by = agentHandler->check_winner();
	    char game_over[1024];
	    snprintf(game_over, 1024, "Player %d: %s has victory! Press <ESC> to exit.", game_won_by, get_plcivname(game_won_by));
	    message(string(game_over));
	    sendMessage(game_over);
	}
	//draw popup menus
	if (draw) player[display->get_active_player()].draw_menu(display, agentHandler);
	// draw resources
    if (draw && !edit_mode && !background_mode) {
        BITMAP* res_ico = data->ui_data.resource_icon;
        int resources = player[active_player].get_resources();
        masked_blit(res_ico, buffer, 0, 0, buffer->w - res_ico->w - 10, buffer->h - res_ico->h - 10, res_ico->w, res_ico->h);
        textprintf_right_ex(buffer, game_font, buffer->w - res_ico->w - 14, buffer->h - text_height(game_font) - 9, makecol(0,0,0), -1, "%d", resources);
        textprintf_right_ex(buffer, game_font, buffer->w - res_ico->w - 15, buffer->h - text_height(game_font) - 10, makecol(255,255,255), -1, "%d", resources);

        // exit button
        if (key[KEY_ESC]) esc_mode = true;
        if (esc_mode) {
            textout_centre_ex(buffer, font, "Leave the game?  (y/n)", buffer->w/2 + 1, buffer->h/3 + 1, makecol(0,0,0), -1);
            textout_centre_ex(buffer, font, "Leave the game?  (y/n)", buffer->w/2, buffer->h/3, makecol(255,255,255), -1);
            if (key[KEY_Y]) end_game();
            else if (key[KEY_N]) esc_mode = false;
        }
    }
	// draw overlayed gui
	display->drawMessages();
	if (draw && !background_mode) display->drawOverlay(this, agentHandler);

	// Scroll
	if (!background_mode) {
        if (mouse_x < SCROLL_WIDTH) display->worldScroll((int)(-12 * (1.0 - mouse_x / SCROLL_WIDTH)), 0);
        if (mouse_x > buffer->w - SCROLL_WIDTH) display->worldScroll((int)(12 * (1.0 - (buffer->w - mouse_x) / SCROLL_WIDTH)), 0);
        if (mouse_y < SCROLL_WIDTH) display->worldScroll(0, (int)(-12 * (1.0 - mouse_y / SCROLL_WIDTH)));
        if (mouse_y > buffer->h - SCROLL_WIDTH) display->worldScroll(0, (int)(12 * (1.0 - ((buffer->h - mouse_y) / SCROLL_WIDTH))));
    }
}

void GAME::update_network(void)
{
    CONNECTION *con;
    MESSAGE *tmp;
    int connected = 0;

    // loop through connections
    for (int i=0; i < network->num_connections(); i++) {
        con = network->get(i);

        // check for disconnect
        if (con->is_connected()) connected++;
        else continue;

        // loop through messages
        while (!con->msg_empty()) {
            tmp = con->msg_tail();
            // handle message
            switch (tmp->type) {
                case MSGTYPE_CHAT:
                    display->addMessage(tmp->msg.chat.str);
                    break;

                case MSGTYPE_TASK:
                    agentHandler->setTasktoSelected(new TASK((TASK::TASK_TYPE)tmp->msg.task.type, tmp->msg.task.pid, tmp->msg.task.a, tmp->msg.task.b, tmp->msg.task.make_current, tmp->msg.task.is_final_dest, tmp->msg.task.group_id));
                    break;

                case MSGTYPE_FIGHT:
                    agentHandler->setTasktoSelected(new TASK((TASK::TASK_TYPE)tmp->msg.fight.type, tmp->msg.fight.pid, agentHandler->find_group(tmp->msg.fight.enemy_pid, tmp->msg.fight.enemy_id), tmp->msg.fight.make_current, tmp->msg.fight.group_id));
                    break;

                case MSGTYPE_GRPSYNC:
                    agentHandler->setTasktoSelected(new TASK(tmp));
                    break;

                case MSGTYPE_NONE:
                    break;
            }

            // finished with message
            con->msg_pop();
        }
    }

    // end game if no one is connected
    if (!connected) {
        gui.display_message("Notification", is_host ? "All players disconnected." : "Host disconnected.");
        end_game();
    }
}

void GAME::mouseInput(void)
{
	// update auto-updated variables that concern the mouse
	curX = display->toWorldX(mouse_x);
	curY = display->toWorldY(mouse_y);
	overlay = (display->isOverOverlay(mouse_x, mouse_y) || gui.is_over(mouse_x, mouse_y));
	mouse_l = mouse_b & 1;
	mouse_r = mouse_b & 2;

	bool overrideTasks = !(key[KEY_LCONTROL] || key[KEY_RCONTROL]);
	bool overMiniMap = display->isOverMiniMap(mouse_x, mouse_y);
	GROUP * gp = NULL;

	// overlay mouse clicks
	if (mouse_l && !alreadyPressedL) overlayClickL = overlay;
	if (mouse_r && !alreadyPressedR) overlayClickR = overlay;

    if (!edit_mode) {
        // Left select
        if (mouse_l && !overlayClickL) {
            if (!alreadyPressedL && !overlay) {
                if (!(click_time < 12 && sqrt((x1 - curX)*(x1 - curX) + (y1 - curY)*(y1 - curY)) < 7)) click_time = 0;
                x1 = curX;
                y1 = curY;
            }
            // drawing selection box would go here if not for the need to draw it after the agents
            if (alreadyPressedL && !(click_time < 12 && click_time && sqrt((x1 - curX)*(x1 - curX) + (y1 - curY)*(y1 - curY)) < 7)) player[active_player].select_area(x1, y1, curX, curY);
        } else if (!mouse_l) click_time++;

        // Right task to location
        if (mouse_r && !overlayClickR) {
            if (!alreadyPressedR) { // task to location on right click
                x1 = curX;
                y1 = curY;
                gp = agentHandler->get_agent_at(curX, curY);
                if (gp && gp->get_team() != get_player()) {
                    agentHandler->setTasktoSelected(new TASK(TASK::FIGHT, get_player(), gp, overrideTasks));
                    is_fight = true;
                } else {
                    agentHandler->setTasktoSelected(new TASK(TASK::MOVE, get_player(), curX, curY, overrideTasks, true));
                    is_fight = false;
                }
            }
        } else if (alreadyPressedR && !mouse_r && !is_fight) { // task to direction on right click release
            if (!overlayClickR && !distIsLessThan(x1 - curX, y1 - curY, DRAG_DIST_TO_ARROW)) agentHandler->setTasktoSelected(new TASK(TASK::ROTATE, get_player(), (int) (atan2(curY - y1, curX - x1) / (2 * M_PI) * 360.0), 0));
        }
    }

	// UI (minimap, buttons) click
	if (mouse_l && overlayClickL) {
		if (!alreadyPressedL || (overMiniMap && (miX != mouse_x || miY != mouse_y) && mtimer >= MINIMAP_TIME_TO_MOVE)) {
			miX = mouse_x;
			miY = mouse_y;
			display->overlayClick(mouse_x, mouse_y);
			mtimer = 0;
		} else mtimer++;
	}

	// UI right click
	if (mouse_r && !alreadyPressedR && overlayClickR && !overMiniMap) {
	    display->overlayClick(mouse_x, mouse_y);
	}

	// minimap on click release
	if (!mouse_l && alreadyPressedL && overlayClickL && overMiniMap) {
		display->overlayClick(mouse_x, mouse_y);
		mtimer = 0;
	}

	// minimap task to location
	if (mouse_r && overMiniMap && !alreadyPressedR) {
		agentHandler->setTasktoSelected(new TASK(TASK::MOVE, get_player(), display->miniMapToWorldX(mouse_x), display->miniMapToWorldY(mouse_y), overrideTasks, true));
	}

	// editor click
	if (edit_mode && !overlay) update_editmode();

	// left mouse button states
	if (mouse_l && !alreadyPressedL) alreadyPressedL = true;
	else if (!mouse_l) alreadyPressedL = false;

	// right mouse button states
	if (mouse_r && !alreadyPressedR) alreadyPressedR = true;
	else if (!mouse_r) {
		alreadyPressedR = false;
		is_fight = false;
	}
}

void GAME::update_editmode(void)
{
    switch (etool.cur_tool) {
        case EDITOR_TOOL::TOOL_TILE : tool_tilebrush();
                break;
        case EDITOR_TOOL::TOOL_OBJ : tool_forest();
                break;
        case EDITOR_TOOL::TOOL_BUILD : tool_building();
                break;
        case EDITOR_TOOL::TOOL_PLACE : tool_units();
                break;
    }

    // delete tool
    tool_erase();

    // clear cur tile between click
    if (!(mouse_b & 3))
        curTileX = curTileY = -1;
}

void GAME::tool_tilebrush(void)
{
    if (mouse_l) { // paint ground

        if (!alreadyPressedL) world->setCurTile(etool.tiles[etool.cur_tile].c_str());

        int dx = curX / TILE_SIZE;
        int dy = curY / TILE_SIZE;
        int px, py;

        if (curTileX != dx || curTileY != dy) {

            curTileX = dx;
            curTileY = dy;

            dx -= etool.brush_size/2;

            if (!etool.brush_shape) {
                dy -= etool.brush_size/2;
                // set tiles (square)
                for (int ix=0; ix<etool.brush_size; ix++) {
                    for (int iy=0; iy<etool.brush_size; iy++) {

                        px = dx + ix;
                        py = dy + iy;

                        // place tiles / delete objects
                        if (px >= 0 && py >= 0 && px < world->getMapWidth() && py < world->getMapHeight()) {
                            if (etool.cur_tool_options) world->editorClearObj(px, py);
                            world->setToCurTile(px, py);
                        }
                    }
                }
                display->redrawTiles(dx, dy, etool.brush_size, etool.brush_size);
            } else {
                dx -= etool.brush_size/2;
                // set tiles (diamond)
                for (int ix=0; ix<etool.brush_size*2 - 1; ix++) {
                    bool odd = ix%2;
                    for (int iy=0; iy<etool.brush_size - odd; iy++) {

                        px = dx + ix + iy - ix/2;
                        py = dy - ix + iy + odd + ix/2;

                        // place tiles / delete objects
                        if (px >= 0 && py >= 0 && px < world->getMapWidth() && py < world->getMapHeight()) {
                            if (etool.cur_tool_options) world->editorClearObj(px, py);
                            world->setToCurTile(px, py);
                        }
                    }
                }
                display->redrawTiles(dx, dy - etool.brush_size, etool.brush_size*2, etool.brush_size*2);
            }

            // update deleted objects screen
            if (etool.cur_tool_options) display->getOverhead();
        }

    } else if (alreadyPressedL) {
        display->updateMiniMap(); // on click release
    }
}

void GAME::tool_forest(void)
{
    if (mouse_l) { // paint ground

        int dx = curX / TILE_SIZE;
        int dy = curY / TILE_SIZE;

        if (world->getPather()->passable_m(curX, curY)) {

            curTileX = dx;
            curTileY = dy;

            // place object / generate forest
            if (etool.brush_size == 1) world->placeObj(curTileX, curTileY, etool.stationaries[etool.cur_stationary].c_str());
            else world->generateForest(curTileX, curTileY, etool.brush_size, etool.cur_tool_options ? etool.brush_size/3 : 0, 1, etool.stationaries[etool.cur_stationary].c_str());

            // update screen
            display->getOverhead();
            display->updateMiniMap();
        }

    }
}

void GAME::tool_building(void)
{
    if (mouse_l) { // paint ground

        int dx = curX / TILE_SIZE;
        int dy = curY / TILE_SIZE;

        if ((curTileX != dx || curTileY != dy) && world->getPather()->passable_m(curX, curY) && !alreadyPressedL) {

            curTileX = dx;
            curTileY = dy;

            // place building
            if (etool.pl > 0)
                tmp_grp = agentHandler->create_building(etool.buildings[etool.cur_building].c_str(), etool.pl, curX, curY);
        }

        // set rotation
        if (tmp_grp && tmp_grp->dist_to(curX, curY) > DRAG_DIST_TO_ARROW) tmp_grp->get_first_agent()->set_rotation(tmp_grp->angle_to(curX, curY));

    } else tmp_grp = NULL;
}

void GAME::tool_units(void)
{
    if (mouse_l) { // paint ground

        int dx = curX / TILE_SIZE;
        int dy = curY / TILE_SIZE;

        if ((curTileX != dx || curTileY != dy) && world->getPather()->passable_m(curX, curY) && !alreadyPressedL) {

            curTileX = dx;
            curTileY = dy;

            // place group
            if (etool.pl > 0)
                tmp_grp = agentHandler->create(etool.units[etool.cur_building].c_str(), etool.pl, curX, curY, etool.brush_size, -1, -1, true);
        }

        // set rotation
        if (tmp_grp && tmp_grp->dist_to(curX, curY) > DRAG_DIST_TO_ARROW) {
			TASK* tt = new TASK(TASK::ROTATE, etool.pl, tmp_grp->angle_to(curX, curY));
			tmp_grp->clear_tasks();
			tmp_grp->add_task(tt);
			delete tt;
		}

    } else tmp_grp = NULL;
}

void GAME::tool_erase(void)
{
    if (mouse_r && !mouse_l) { // paint ground

        int dx = curX / TILE_SIZE;
        int dy = curY / TILE_SIZE;

        if (!world->getPather()->passable_m(curX, curY)) {
            // erase location
            if (world->editorClearObj(dx, dy)) {
                display->getOverhead();
                display->updateMiniMap();
            }
        }

        // destroy group
        GROUP* gp = agentHandler->get_agent_at(curX, curY);
        if (gp) gp->die();
    }
}

void GAME::tool_tilefill(void)
{
    world->setCurTile(etool.tiles[etool.cur_tile].c_str());

    world->tile_fill();

    display->worldGoto(display->getWorldX(), display->getWorldY());
    display->updateMiniMap();
}

bool GAME::save_map(const char * fname)
{
    // open file
    FILE* fout = fopen(fname, "w");
    if (!fout) {
        perror("fopen()");
        printf("problem saving map as '%s'\n", fname);
        return false;
    }

    // the magic header
    fprintf(fout, "MapFile [http://wotwar.googlecode.com]\n");
    fprintf(fout, "%d %d\n", world->getMapWidth(), world->getMapHeight());
    fprintf(fout, "%d\n\n", starting_resources);

    // save player factions
    save_factions(fout);

    // save tile map
    world->save_tiles(fout);

    // save stationaries
    world->save_stationaries(fout);

    // save buildings
    agentHandler->save_agents(fout);

    fclose(fout);
    return true;
}

void GAME::save_factions(FILE * fout)
{
    fprintf(fout, "%d\n", num_players);

    for (int i=0; i<num_players; i++)
        fprintf(fout, "%d\n", get_plciv(i+1));

    fputs("end\n", fout);
}

const char * GAME::get_plcivname(int pl)
{
    return data->get_civname(player[pl].get_faction());
}

int GAME::get_plciv(int pl)
{
    return player[pl].get_faction();
}

void GAME::set_plciv(int pl, int to_civ)
{
    player[pl].set_faction(to_civ);
}

void GAME::draw_loadbar(BITMAP* bmp, int x, int y, int width, int height, int border_color, int color, int percent)
{
	// setup
	int barw = width * percent / 100;
	if (barw >= width) barw = width;
	if (barw <= 0) barw = 1;
	int nx = x - width/2;
	int ny = y - height/2;

	y -= text_height(font) / 2;

	// draw
	rect(bmp, nx, ny, nx + width, ny + height, border_color);
	rectfill(bmp, nx + 1, ny + 1, nx + barw - 1, ny + height - 1, color);
	textout_centre_ex(buffer, font, "Loading...", x - 1, y - 1, color, -1);
	textout_centre_ex(buffer, font, "Loading...", x, y, makecol(0,0,0), -1);
}

void GAME::init_players(void)
{
	player.resize(num_players+1);
	for (int i=0; i<player.size(); i++) player[i].init(i, 0, starting_resources);

	player[active_player].set_active();
	players_alive = num_players;
}

int GAME::get_player(void)
{
	return active_player;
}

int GAME::get_num_players(void)
{
	return num_players;
}

AGENT_HANDLER * GAME::get_agent_handler(void)
{
    return agentHandler;
}

void GAME::message(string msg)
{
    display->addMessage(msg.c_str());
}

bool GAME::is_loaded(void)
{
    return (game_loaded && !is_editmode());
}

int GAME::get_startingresources(void)
{
    return starting_resources;
}

void GAME::set_startingresources(int r)
{
    starting_resources = r;
}

void GAME::generate_battle(void)
{
    if (agentHandler->size() > 1 || num_players < 2) return;

    const char * utype = data->get_random_unit();
    if (!utype) return;

    bool dir = false;
    int angle = rand()%90 - 45;
    int x = world->getWidth()/2;
    int y = world->getHeight()/2;
    int dist = sqrt((buffer->w-400)*(buffer->w-400) + buffer->h*buffer->h)/2;
	int mx, my;

    int pl = (player[1].units_alive() > 0) ? 2 : 1;

    GROUP* cl = agentHandler->get_closest_group(x, y);

    // come from opposite directions
    mx = x;
	my = y;
	
	if (cl) {
        if (cl->get_x() > x) dir = true;
		mx = cl->get_x();
		my = cl->get_y();
    }

    if (dir) {
        angle += 180;
        x -= 200;
		mx -= 350;
    } else {
        x += 200;
		mx += 350;
    }

	if (!cl) {
		mx = x;
		my = y;
	}
    
	// skip all the mx, my stuff
	mx = x;
	my = y;

    GROUP* gp = agentHandler->create(utype, pl, x + cos(M_PI * angle/180.)*dist, y + sin(M_PI * angle/180.)*dist, rand()%6 + 3);
    if (gp) {
        TASK* t = new TASK(TASK::MOVE, gp->get_team(), mx, my, true, true);
        gp->add_task(t);
        delete t;
    }
}

