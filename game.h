#ifndef GAME_H
#define GAME_H

#include "wotwar_classes.h"
#include <vector>
#include <string>
#include <cstdio>

class EDITOR_TOOL
{
    public:
        EDITOR_TOOL(void);
        enum {TOOL_NONE=0, TOOL_TILE, TOOL_OBJ, TOOL_BUILD, TOOL_PLACE, TOOL_FILL};
        void draw_tools(DISPLAY * display, int worldx, int worldy);
        int players;
        vector<string> tiles;
        vector<string> stationaries;
        vector<string> buildings;
        vector<string> units;
        vector<string> civs;
        int cur_tile, cur_stationary, cur_building;
        int brush_size, brush_shape, pl;
        int cur_tool, cur_tool_options;
        char name[256];
};

class GAME
{
	private:
		DISPLAY* display;
		WORLD* world;
		GUI& gui;
		BITMAP*& buffer;
		SCREEN& vscreen;
		AGENT_HANDLER* agentHandler;
		PARTICLE_SYSTEM* particle_sys;
		ATTACK_DATA* attack_data;
		DATA* data;
		GROUP* tmp_grp;
		bool background_mode;
		bool failed_load;
		bool is_host;
		bool esc_mode;
		int game_won_by;
		int starting_resources;

        // editor
		bool edit_mode;
		EDITOR_TOOL etool;
		void setup_editor(void);
		void draw_ltext(const char * str, const char * str2 = "");

		// players
		int active_player;
		int num_players;
		void init_players(void);
		int players_alive;

		// networking
		string host_name;
		NETWORK * network;
		bool host_tx_file(string mapname);

		// auto-updated variables storing states
		int x1, y1; // mouse click-and-drag, world point where first pressed
		int curX, curY; // mouse current world X and Y coordinates
		int curTileX, curTileY;
		bool alreadyPressedL; // mouse left button being held down flag
		bool alreadyPressedR; // ditto for the right button
		bool overlay; // is mouse over game or gui (flag)
		bool mouse_l, mouse_r; // mouse button states
		bool overlayClickL, overlayClickR; // mouse button was initially pressed over the GUI
		bool is_fight;
		int miX, miY, mtimer, click_time; // minimap timer and previous x and y
		bool game_loaded;

	public:
		vector<PLAYER> player;

        AGENT_HANDLER * get_agent_handler(void);
		int get_player(void);
		int get_num_players(void);
		EDITOR_TOOL * get_etool(void);
		bool is_editmode(void);
		void update(bool draw=true);
		void update_network(void);
		bool is_failed(void);
		bool is_loaded(void);
		void end_game(void);
		void sendMessage(char *str);

        // map editor
		void update_editmode(void);
		void tool_tilebrush(void);
		void tool_tilefill(void);
		void tool_forest(void);
		void tool_building(void);
		void tool_units(void);
		void tool_erase(void);
		void draw_tools(void);
		bool save_map(const char * fname);
		void save_factions(FILE * fout);
		void message(string msg);
		FILE * open_header(const char * fname);
		void load_players(FILE * fin);
		bool load_check(FILE * fin);
		int get_startingresources(void);
		void set_startingresources(int r);

		int get_plciv(int pl);
		const char * get_plcivname(int pl);
		void set_plciv(int pl, int to_civ);

		// background menu
        void generate_battle(void);

		void mouseInput(void);
		void draw_loadbar(BITMAP* bmp, int x, int y, int width, int height, int border_color, int color, int percent);
		GAME(BITMAP*& buffer, SCREEN& vscreen, UI_DATA& ui_data, GUI& gui, int num_players, int player_id, const char * map_file, int size_map, bool edit_mode=false, bool is_host=false, bool enable_networking=true, const char * hostname="");
		virtual ~GAME(void);
};

#endif
