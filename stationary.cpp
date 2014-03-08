#include "wotwar.h"

void STATIONARY::setPerm(void)
{
	tmp = 1;
}

STATIONARY::STATIONARY(void) : tmp(1), img(NULL), collide_node(NULL)
{

}

STATIONARY::~STATIONARY(void)
{
	if (img && !tmp) destroy_bitmap(img);
	if (collide_node) delete collide_node;
}

void STATIONARY::set(BITMAP *bitmap)
{
	if (img = bitmap) depth = (img->w + img->h) / 2;
}

void STATIONARY::addCollisions(int x, int y, PATHER* pather)
{
	if (!collide_node) collide_node = new COLLISION(this);
	if (img) {
	    pather->setCircle(x, y, (double)STATIONARY_SIZE/2, collide_node);
	    pather->set_stationary_m(x, y);
	}
}
