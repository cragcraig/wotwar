#ifndef GROUP_H
#define GROUP_H

#include "wotwar_classes.h"
#include "tasklist.h"
#include <vector>

class GROUP : public MOVE
{
	public:
		enum STATE {NORMAL, MOVING, IDLE, FIGHTING, RANGED_FIGHTING, UPDATE_FORMATION, BUILDING, CREATING_BUILDING};

	private:
		list<AGENT> agents;
		list<list<AGENT>::iterator> agents_draw_order;
		list<list<AGENT>::iterator> zombies;
		list<TASK*> linked_tasks;
		list<GROUP*> attackers;

		AGENT * template_agent;
		AGENT_HANDLER * agent_handler;
		UNIT_DATA * unit_data;
		ANIMATION_DATA * ani_data;
		BUILDING_DATA * build_data;
		const char * buildplot;
		const char * parent_type;
		bool zombie;
		bool disable_fighting;
		const ATTACK_TYPE * attack_type;
		PLAYER * player;
		TASKLIST task_list;
		VEC * agent_pos;

		int check_x, check_y;
		int build_x, build_y;
		int gid;
		int interrupted_task;
		int agent_id;

		bool sel;
		STATE state;
		int team_id;
		int team_color;
		const int max_units;
		int num_units;
		int units_in_place;
		int units_collided;
		int recheck_counter;
		int max_unit_dist;
		int group_radius;
		int group_width;
		bool find_formation;
		bool building_block;
		int nearby_check_counter;
		int resource_counter;

		int gen_pathfind(int level);
		void units_to_group_rotation(void);
		void find_ideal_formation(void);
		void flip_formation(void);
		bool is_new_task(void);
		void delete_agents(void);
		void compute_average_location(int& xs, int& ys);
		void set_location_to_unit_avg(bool also_dest=false);
		double dist_from_agent_avg(void);
		void revert_interrupted(void);

		void clear_status(void);
		void update_building(void);
		void update_buildplots(void);
		void building_tasks(void);
		void update_group(void);
		void update_self(void);
		void update_tasks(void);
		void update_deaths(void);
		bool check_nearbyenemies(void);

		void update_agents_idle(void);
		void update_agents_formation(void);
		void update_agents_fight(void);
		void update_agent(list<AGENT>::iterator& p, int index);
		void point_agents_dest(void);
		void replace_building(bool create_new, int team=-1);

		// send network sync
		void sync_initmsg(MESSAGE* msg);
		void sync_grpsync(void);
		void sync_attack(int player, int target_id);
		void sync_buildplot(int build_player);
		void sync_die(void);

		void unlink_all(void);

    public:
		GROUP(AGENT_HANDLER * ahp, UNIT_DATA * udp, ANIMATION_DATA * adp, BUILDING_DATA * bdp, PATHER * pp, PARTICLE_SYSTEM& part_sys, ATTACK_DATA& attack_data, int team, int max, double x, double y, const char * parent=NULL, double from_x=-1, double from_y=-1, bool disable_fighting=false);
		virtual ~GROUP(void);
		const char * get_type(void);
		const char * get_parenttype(void);
		void set_team(int team);
		int get_team(void);
		int get_group_id(void);
		int get_size(void);
		int get_team_color(void);
		int get_radius(void);
		GROUP::STATE get_state(void);
		vector<BUILDABLE>* get_buildable(void);
		void update(void);
		void die(void);
		void sort_units(void);
		void select(bool s = true);
		bool is_sel(void);
		bool is_single_unit(void);
		bool is_building(void);
		bool is_buildsite(void);
		bool is_fightable(void);
		bool is_near_agent(int x, int y);
		int dist_to_agent(int x, int y);
		void add_task(TASK* tp);
		void add_task_front(TASK * tp);
		void clear_tasks(void);
		TASKLIST * get_tasklist(void);
		TASK::TASK_TYPE task_type(void);
		void add_units(int n=1, double x1=-1, double y1=-1);
		void kill_unit(list<AGENT>::iterator& up);
		void drawToMiniMap(DISPLAY* display, int x, int y, double scale);
		void draw_path_minimap(DISPLAY* display, int x, int y, double scale);
		void draw_task_path(DISPLAY* display);
		void draw(DISPLAY* disp);
		void draw_selected(DISPLAY* disp);
		void draw_minmap_buildpoint(DISPLAY* display, int x, int y, double scale);
		void sync_network(MESSAGE* msg);

        void generate_ideal_formation(void);
		void warn_attack(GROUP * gp);
		AGENT * find_closest(double nx, double ny);
		AGENT * find_random_agent(int seed);

        AGENT * get_first_agent(void);
		AGENT * find_unit(int unit_id);
		AGENT * find_nextunit(int unit_id);

		bool check_insidegroup_collision(double x, double y, unsigned int id, int& xhit, int& yhit);
		int check_insidegroup_collision3(double x, double y, double xprev, double yprev, double speed, unsigned int id, int& xhit, int& yhit);
		int check_insidegroup_collision2(double x, double y, double xprev, double yprev, unsigned int id);

		void link_task(TASK * t);
		void unlink_task(TASK * t);

		GROUP* get_opponent();
		void add_attacker(GROUP* g);
		void remove_attacker(GROUP* g);
		list<GROUP*>* get_attackers();

		static bool sorter(AGENT& a1, AGENT& a2);

		GROUP * pPrevious, * pNext;
};

#endif
