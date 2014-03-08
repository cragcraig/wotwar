#ifndef PLAYER_H
#define PLAYER_H

#include "wotwar_classes.h"
#include "selector.h"

class PLAYER
{
	private:
		int new_color(int id);
		void clear_select(void);
		SELECTOR sel;
		int color;
		int id;
		int faction;
		int group_id_count;
		bool active;
		int unit_count;
		bool can_die;
		int resources;
		list<GROUP*> selected;

	public:
		enum PLAYER_TYPES {ANY_PLAYER=-1, WORLD_PLAYER=0};
		void init(int id, int faction, int start_resources=0);
		int get_color(void);
		int get_faction(void);
		void set_faction(int to_civ);
		int new_group_id(void);
		bool is_active(void);
		bool is_host(void);
		void set_active(void);

		void select_area(int x1, int y1, int x2, int y2);
		void check_select(GROUP* pAgent);
		void update_select(void);

		void draw_selected(DISPLAY* disp);
		void draw_menu(DISPLAY* disp, AGENT_HANDLER* ah);
		void add_selected(GROUP * ap);
		void remove_selected(GROUP * ap);

		void unit_decrement(void);
		void unit_increment(void);
		int units_alive(void);
		bool is_dead(void);

		void add_resources(int n);
		void subtract_resources(int n);
		int get_resources(void);

		GROUP* get_selected_building(void);

		POPUP* popup;

		PLAYER(void);
		~PLAYER(void);

};


#endif
