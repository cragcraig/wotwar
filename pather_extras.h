#ifndef PATHER_EXTRAS_H
#define PATHER_EXTRAS_H

#include "wotwar_classes.h"

class COLLISION { // holds pointers for a collideable object
	public:
		AGENT* pA;
		STATIONARY* pS;
		int x, y;
		void set(AGENT* agent) {pA = agent; pS = NULL;}
		void set(STATIONARY* stationary) {pS = stationary; pA = NULL;}
		bool occupied(void) { return pS || pA; }
		COLLISION(AGENT* agent) : pA(agent), pS(NULL), x(-1), y(-1) {}
		COLLISION(STATIONARY* stationary) : pA(NULL), pS(stationary), x(-1), y(-1) {}
};

#endif
