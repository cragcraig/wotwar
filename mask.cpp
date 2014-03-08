#include "mask.h"

void MASK::delMask(void)
{
	if (m) delete m;
	m = NULL;

	if (m1) delete m1;
	m1 = NULL;
}

void MASK::clearMask(void)
{
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			m[x*h + y] = OPEN;
			m1[x*h + y] = OPEN;
		}
	}
}

void MASK::set(int x, int y, char c)
{
	m[x*h + y] = c;
	m1[x*h + y] = c;
}

char MASK::check(int x, int y, char c)
{
	return m[x*h + y];
}

char MASK::check_boundscheck(int x, int y)
{
	if (x >= 0 && y >= 0 && x < w && y < h) return m[x*h + y];
	else return BLOCK;
}

void MASK::set_to_boundscheck(int x, int y, char c)
{
	if (x >= 0 && y >= 0 && x < w && y < h) {
	    m[x*h + y] = c;
	    m1[x*h + y] = c;
	}
}

void MASK::set_boundscheck(int x, int y, char c)
{
	if (x >= 0 && y >= 0 && x < w && y < h) {
		char * p = &m[x*h + y];
		char * p1 = &m1[x*h + y];
		if (c == BLOCK || (*p1 != BLOCK && c > *p1)) {
		    *p = c;
		    *p1 = c;
		}
	}
}

void MASK::tmp_increase(int x, int y)
{
	if (x >= 0 && y >= 0 && x < w && y < h) {
		char * p = &m[x*h + y];
		if (*p == MASK::OPEN) *p = MASK::BAD;
		else if (*p == MASK::BAD) *p = MASK::BLOCK;
	}
}

void MASK::tmp_set(int x, int y, char c)
{
	if (x >= 0 && y >= 0 && x < w && y < h) {
		m[x*h + y] = c;
	}
}

void MASK::tmp_reset(int x, int y)
{
	if (x >= 0 && y >= 0 && x < w && y < h) {
		m[x*h + y] = m1[x*h + y];
	}
}

void MASK::initMask(int w, int h, int areasize, int level)
{
	delMask();
	m = new char[w * h];
	m1 = new char[w * h];
	this->w = w;
	this->h = h;
	this->areasize = areasize;
	this->level = level;
	clearMask();
}

void MASK::closest_open(int x, int y, int &nx, int &ny)
{
	int r=0;
	while (1) {
		ny = y + r;
		for (nx=x-r; nx < x+r; nx++) if (check_boundscheck(nx, ny) != BLOCK) return;
		ny = y - r;
		for (nx=x+r; nx > x-r; nx--) if (check_boundscheck(nx, ny) != BLOCK) return;
		nx = x + r;
		for (ny=y+r; ny > y-r; ny--) if (check_boundscheck(nx, ny) != BLOCK) return;
		nx = x - r;
		for (ny=y-r; ny < y+r; ny++) if (check_boundscheck(nx, ny) != BLOCK) return;
		r++;
	}
}

MASK::MASK(void)
{
	m = NULL;
	m1 = NULL;
}

MASK::MASK(int w, int h, int areasize, int level)
{
	m = NULL;
	m1 = NULL;
	initMask(w, h, areasize, level);
}

MASK::~MASK(void)
{
	delMask();
}
