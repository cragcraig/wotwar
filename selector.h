#ifndef SELECTOR_H
#define SELECTOR_H

struct SELECTOR
{
	bool select;
	int selectX, selectY, selectBoxX, selectBoxY;
	double selectDist;
	double tmpDist;
	GROUP* agent_to_select;
};

#endif
