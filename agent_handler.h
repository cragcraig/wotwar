#ifndef AGENT_HANDLER_H
#define AGENT_HANDLER_H

#include "wotwar_classes.h"
#include <cstdio>

class AGENT_HANDLER
{
	private:
		unsigned int length;
		vector<GROUP*> toKillList;
		DISPLAY* display;
		PARTICLE_SYSTEM& part_sys;
		ATTACK_DATA& attack_data;
		GROUP* pFirst;
		GROUP* pLast;
		TASK* taskForSelected;
		TASK* taskForSelectedNextLoop;
		DATA* data;
		PATHER* pather;
		NETWORK* network;
		bool is_paused;
		vector<PLAYER>& player;
		void add(GROUP* agent);
		void add_start(GROUP* agent);
		void killList(void);
		void killNow(GROUP* agent);
		void network_sndtask(TASK* task, GROUP* agent);

		void multiple_agents_move_task(TASK* t, GROUP* a);
		void shiftOffTasks(void);
		void give_agent_tasks(GROUP* pAgent);
		void debug_unit_creator(void);

	public:
		void setTasktoSelected(TASK* task); // this loop (name should be changed to setTask)
		void setTaskNextLoop(TASK* task); // next loop
		void kill(GROUP* agent);
		void killAll(void);
		void updateAll(bool draw=true);
		void drawAllToMiniMap(int x, int y, double scale);
		void pause(bool set);
		void send_broadcast(MESSAGE* msg);
		WORLD* get_world(void);
		GROUP* get_agent_at(int x, int y);
		GROUP* get_agent_within(int x, int y, int dist, GROUP* agent_ignore = NULL);
		GROUP* get_building(int team);
		GROUP* get_closest_group(int x, int y, int farthest_dist=9999, int team=-1);
		GROUP* get_closest_nonteamgroup(int x, int y, int farthest_dist, int team);
		PLAYER* get_player(int id);
		int size(void) {return length;}
		bool check_close_behind(GROUP* g, int dist);

		int check_winner(void);

		GROUP* create(const char * typeName, int team, int x, int y, int size, int from_x=-1, int from_y=-1, bool disable_fighting=false);
		GROUP* create_building(const char * typeName, int team, int x, int y, const char * parent=NULL);

        GROUP * find_group(int player, int group_id);
        GROUP * find_group(int move_id);

        PLAYER* get_player_pointer(int player_id);
        DATA * get_data(void);

        void save_agents(FILE * fout);
        void load_agents(FILE * fin);

		AGENT_HANDLER(NETWORK* network, DISPLAY* display, PATHER* pather, DATA* data, PARTICLE_SYSTEM& part_sys, ATTACK_DATA& attack_data, vector<PLAYER>& player_list);
		virtual ~AGENT_HANDLER(void);
};

#endif
