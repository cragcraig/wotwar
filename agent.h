#ifndef AGENT_H
#define AGENT_H

#include "wotwar_classes.h"
#include "data_types.h"
#include "move.h"
#include "sprite.h"

class AGENT : public MOVE, public SPRITE
{
	public:
		enum STATE {NORMAL, FIGHTING, READY_FIGHT};

	private:
		unsigned int group_id;
		int sort_id;
		double health;
		bool zombie, highlight;
		bool rotate_to_group;
        int fight_timer;
        PARTICLE_SYSTEM& part_sys;
        BUILDING_DATA * build_data;
        const ATTACK_TYPE * attack_type;
		int stuckcounter;

		int last_swap, swap_count;
		STATE state;

		UNIT_DATA * unitData;
		LINK<AGENT> target;

		bool at_unreachable_dest(void);
		void swap_positions(AGENT * ap);
		void check_health(void);
		void die(void);

		list<LINK<AGENT>*> linked;

		int check_attackcollisions(int& xhit, int& yhit);
        AGENT * find_target(GROUP * target_group);
        AGENT * find_target_random(GROUP * target_group);
		void set_target(AGENT * ap);
		void end_target(void);
		void unlink_all(void);

		void update(void);

	public:
		void draw(DISPLAY* display);
		void draw_selected(DISPLAY* display);
		void update_building(void);
		void update_normal(void);
		void update_fight(GROUP* target_group);
		void update_rangedfight(GROUP* target_group);

		GROUP * get_group_pointer(void);
		void set_data(UNIT_DATA * data, BUILDING_DATA * bdata);
		UNIT_DATA * get_data(void);
		int get_team(void);
		unsigned int getGroup(void);
		char * getType(void);
		bool is_zombie(void);
		bool is_fighting(void);

		void hurt(double damage);
		void hurt_by(double damage, AGENT * ap);
		void hurt_by(double damage, GROUP * gp);
		void set_health(double health);
		void add_health(double health);
		double get_health(void);

		int get_sort_id(void);
		void set_sort_id(int sid);
		bool is_last_swap(int sid);
		void update_swap(void);
		bool check_fight_collisions();
		bool is_highlighted() {return highlight;}
		void set_highlighted(bool s) {highlight = s;}
		void clear_stuckcounter() {stuckcounter = 0;}
		bool is_stuckcounter() {return stuckcounter > 8;}

		bool start_rotate_to_group(void);
		bool end_rotate_to_group(void);

        bool is_team_collision(void);
		bool isOnScreen(DISPLAY* display);
		int getRadius(void);
		void drawToMiniMap(DISPLAY* display, int x, int y, double scale);

		void link(LINK<AGENT> * t);
		void unlink(LINK<AGENT> * t);

		AGENT(GROUP * gp, UNIT_DATA * unitData, ANIMATION_DATA * animationData, BUILDING_DATA * bdp, PATHER * pp, PARTICLE_SYSTEM& part_sys, const ATTACK_TYPE * attack_type, int x, int y);
		virtual ~AGENT(void);
};

#endif
