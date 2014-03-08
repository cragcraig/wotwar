#ifndef TASK_H
#define TASK_H

#include "message.h"

class UNIT_DATA;

class TASK
{
	public:
		enum TASK_TYPE {NONE, MOVE, ROTATE, WAIT, WAIT_SIG, SIGNAL, CHECK_WAIT, FIGHT, WAIT_COLLISION, CREATE_BUILDING, BUILD, BUILD_BUILDING, UNBUILD, SKIP, GRPSYNC};
		int x, y, theta, groupX, groupY, count, pathfind_level, build_id;
		bool makeCurrent, startCount, message_only, checked_flag, done_flag, sig_flag, gettingInFormationFlag, is_pathfind, replace_next, linked_flag, build_flag, is_final;
		UNIT_DATA* unit_data;
		GROUP* attacklisted;
		GROUP* link;
		int groupUnits;
		int player;
		int num_assigned, radius;
		int counter, generic_counter, buildtime;
		unsigned int id, pId;
		int specific_group;
		TASK_TYPE type;
		TASK * pNext;

		static unsigned int newId(void); // gives every task a unique ID
		static unsigned int newPathId(void); // generates a unique path ID

		void unlink(void);
		void relink(void);

		void fit_to_map(WORLD * w);

		MESSAGE net_msg;

		TASK(void);
		TASK(TASK::TASK_TYPE t, int player_id, int a=0, int b=0, bool make_current=false, bool is_final_dest=false, int specific_group=0); // most tasks
		TASK(TASK::TASK_TYPE t, int player_id, GROUP * gp, bool make_current=false, int specific_group=0); // fight task
		TASK(MESSAGE * message); // send a network message
		~TASK(void);
};

#endif
