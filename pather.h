#ifndef PATHER_H
#define PATHER_H

#include "wotwar_classes.h"
#include "task.h"
#include "mask.h"
#include "pather_extras.h"
#include "stationary.h"

class PATHER { // handles collisions, pathfinding, etc.
	private:
		WORLD* world;
		COLLISION** mask;
		int width, height, areaWidth, areaHeight;
		MASK smask; // small area mask

		// generic collision object for buildings
		STATIONARY generic_obj;
		COLLISION generic;

	public:
		// Small area collision functions
		enum COLLISION_TYPE {NO_COLLIDE=0, AGENT_COLLIDE, STATIONARY_COLLIDE};
		int patchSize;
		int mapsearchnode_error;
		bool passable(int x, int y);
		bool passableStationary(int x, int y);
		bool passable_unit(int x, int y, COLLISION * col);
		bool passableIgnoreTeam(int x, int y, int teamIgnore);
		bool passableIgnoreGroup(int x, int y, int groupIgnore);
		PATHER::COLLISION_TYPE collision_type(int x, int y);
		AGENT* check_agent_collision(int x, int y);
		STATIONARY* check_stationary_collision(int x, int y);
		void clear(int x, int y);
		void clear(int x, int y, COLLISION * col);
		void set(int x, int y, COLLISION * col);
		void set_autounset(int x, int y, COLLISION * col);

		bool is_on_map(int x, int y);

		AGENT* who_collision(int x, int y);

		// Small area collision shape functions
		void setBox(int x1, int y1, int x2, int y2, COLLISION * col = NULL);
		void setCircle(int x, int y, double r, COLLISION * col = NULL);
		void clearBox(int x1, int y1, int x2, int y2);
		void clearCircle(int x, int y, double r);
		void setBlock(int x, int y, COLLISION * col);
		void clearBlock(int x, int y, COLLISION * col);

		// Pathfind (medium) area collision functions
		void set_box_m(int x1, int y1, int x2, int y2);
		void set_stationary_m(int x, int y);
		bool passable_m(int x, int y);
		bool is_open(int x, int y);
		void make_open(int x, int y);
		void set_blocked(int x, int y);
		void set_notblocked(int x, int y);
		int get_pathfind_areasize(void);

		// add/remove buildings to pathfinding and collisions
		void add_building_m(int x, int y);
		void remove_building_m(int x, int y);

		// resets
		void clearAll(void);
		void clearMask(void);
		void clearMask_m(void);

		// find paths
		TASK* get_path(int x, int y, int fx, int fy);
		TASK* get_path_closest(int x, int y, int fx, int fy);

		// arrange multiple group
		void group_arrange(int num, int &nx, int &ny);

		PATHER(WORLD* world);
		virtual ~PATHER(void);
};

#endif
