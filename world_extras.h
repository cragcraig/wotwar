#ifndef WORLD_EXTRAS_H
#define WORLD_EXTRAS_H

#include "wotwar_classes.h"

class TILE { // holds data for a loaded tile
	public:
		BITMAP* texture;
		char name[MAX_TILE_NAME_LENGTH], type[MAX_TILE_NAME_LENGTH];
		int depth;
		bool tmp;
		bool blocked;
		void setPerm(void) { tmp = 1; }
		TILE(void) : tmp(1), texture(NULL), blocked(false) { }
		virtual ~TILE(void) {
			if (texture && !tmp) destroy_bitmap(texture);
		}
};

class COORDINATE { // holds a coordinate location - used for array of onscreen stationary objects
	public:
		int x, y, depth;
		bool operator>(COORDINATE& o) { return this->depth > o.depth; }
		bool operator<(COORDINATE& o) { return this->depth < o.depth; }
		COORDINATE(void) {}
		COORDINATE(int x, int y, int d=0) : x(x), y(y), depth(d) { }
};

#endif
