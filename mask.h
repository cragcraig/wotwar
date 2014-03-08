#ifndef MASK_H
#define MASK_H

#include "wotwar_classes.h"

class MASK // holds a mask level for use with A* pathfinding
{
	public:
		char* m;
		char* m1;
		int w, h, areasize, level;
		enum PASS_TYPE {BLOCK=0, OPEN=1, BAD=10, REALLY_BAD=25};
		void delMask(void);
		void clearMask(void);
		void set(int x, int y, char c);
		char check(int x, int y, char c);
		char check_boundscheck(int x, int y);
		void set_to_boundscheck(int x, int y, char c);
		void closest_open(int x, int y, int &nx, int &ny);
		void set_boundscheck(int x, int y, char c);
		void initMask(int w, int h, int areasize, int level);

		void tmp_increase(int x, int y);
		void tmp_set(int x, int y, char c);
		void tmp_reset(int x, int y);

		MASK(void);
		MASK(int w, int h, int areasize, int level);
		~MASK(void);
};

#endif
