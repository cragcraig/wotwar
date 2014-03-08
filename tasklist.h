#ifndef TASKLIST_H
#define TASKLIST_H

#include "wotwar_classes.h"

class TASKLIST {
	public:
		unsigned int length;
		TASK* pFirst;
		TASK* pLast;

		AGENT_HANDLER* pAH;
		void add(TASK* task);
		TASK* insert(TASK* link, TASK* task); // insert task after link and before link->pNext
		void endCurrent(void);
		void dropAll(void);
		void drop_path_tasks(void);
		TASK* insertList(TASK* link, TASK* task); // insert a list of tasks after link and before link->pNext
		TASK* add_front(TASK* task);
		GROUP* find_next_attack(void);
		void drop_until_attack(void);

		void count_builds(int ret_array[]);

		void init(AGENT_HANDLER* pAH) {this->pAH = pAH;}

		inline TASK* current(void) {
			return pFirst;
		}

		inline TASK* last(void) {
		    return pLast;
		}

		inline bool nextIsMove(void) {
			if (pFirst->pNext && pFirst->pNext->type != TASK::NONE) return true;
			else return false;
		}

		inline TASK::TASK_TYPE currentType(void) {
			if (pFirst != NULL) return pFirst->type;
			else return TASK::NONE;
		}

		TASKLIST(void);
		virtual ~TASKLIST(void);
};

#endif
