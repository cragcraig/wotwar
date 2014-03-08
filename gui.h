#ifndef GUI_H
#define GUI_H

#include "wotwar_classes.h"
#include <string>
#include <list>

class BUTTON
{
    public:
        BUTTON(int x, int y, const char* txt, int* flag, int val);
        int is_over(int ox, int oy, bool already_pressed=true);
        void draw(BITMAP* buffer);
        int x, y, w, h, val;
        int* flag;
        const char* txt;
};

class GUI
{
	private:
		GAME * game;
		DIALOG * dialog;
		int but_flag;
		SCREEN& screen;
		BITMAP*& buffer;
		UI_DATA ui_data;
		bool center_it;
		bool already_pressed;
		bool in_mainmenu;
		string title;
		list<BUTTON> buttons;

		int run_dialog(DIALOG * d, bool center, int focus=-1);

	public:
		GUI(const char * title, SCREEN& screen);
		~GUI(void);

		void new_game(void);
		void join_game(void);
		void new_gameedit(int size, int players, bool load=false);
		void new_backgroundgame(void);
		void end_game(void);
		bool is_ingame(void);
		void display_dialog(DIALOG * d, bool center);
		void display_message(const char * title, const char * msg);
		void update(bool draw=true);

        void message(const char * title, const char * msg);
		void main_menu(void);
		void map_editor(void);

		void tile_editor(void);
		void object_editor(void);
		void building_editor(void);
		void unit_editor(void);
		void civ_editor(void);
		void save_dialog(void);
		void resources_dialog(void);
		const char * load_dialog(void);
		std::string text_dialog(const char * title, const char * start_val = NULL);

		bool is_over(int x, int y);

		void draw_buttons(void);
		void add_button(int x, int y, const char* txt, int val);
        void clear_buttons(void);
};

#endif
