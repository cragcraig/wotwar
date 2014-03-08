#ifndef STATIONARY_H
#define STATIONARY_H

#include "wotwar_classes.h"

class STATIONARY { // holds data for a loaded stationary object
	public:
		BITMAP* img;
		char type[MAX_TILE_NAME_LENGTH];
		int depth;
		bool tmp;
		COLLISION * collide_node;
		void set(BITMAP *bitmap);
		void addCollisions(int x, int y, PATHER* pather);
		void setPerm(void);
		STATIONARY(void);
		virtual ~STATIONARY(void);
};

#endif
