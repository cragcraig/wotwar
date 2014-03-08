#include "wotwar.h"

PATHER::PATHER(WORLD* world) : generic(&generic_obj)
{
	this->world = world;
	patchSize = PATCH_SIZE;
	width = (int)ceil(world->getWidth()/(double)PATCH_SIZE);
	height = (int)ceil(world->getHeight()/(double)PATCH_SIZE);

	// Small Mask
	mask = new COLLISION*[width * height]; // allocate mask memory
	// clear patch mask
	for (int y=0; y<height; y++) {
		for (int x=0; x<width; x++) {
			mask[y*height + x] = NULL;
		}
	}

	// Area mask
	smask.initMask((int)ceil(world->getWidth()/(double)SMALL_AREA_SIZE), (int)ceil(world->getHeight()/(double)SMALL_AREA_SIZE), SMALL_AREA_SIZE, 1);
}

PATHER::~PATHER(void) {
	delete [] mask;
}

void PATHER::clearAll(void)
{
    clearMask();
    clearMask_m();
}

// set, clear, or check small areas

// check if passible
bool PATHER::passable(int x, int y) {
	x /= PATCH_SIZE;
	y /= PATCH_SIZE;
	return (x >= 0 && y >= 0 && x < width && y < height && !mask[y*height + x]);
}

bool PATHER::passableStationary(int x, int y) { // ignore units
	x /= PATCH_SIZE;
	y /= PATCH_SIZE;
	return (x >= 0 && y >= 0 && x < width && y < height && (!mask[y*height + x] || !mask[y*height + x]->pS));
}

bool PATHER::passableIgnoreTeam(int x, int y, int teamIgnore) {
	x /= PATCH_SIZE;
	y /= PATCH_SIZE;
	COLLISION* tmp = mask[y*height + x];
	return (x >= 0 && y >= 0 && x < width && y < height && (!tmp || (tmp->pA && tmp->pA->get_team() == teamIgnore)));
}

bool PATHER::passableIgnoreGroup(int x, int y, int groupIgnore) {
	x /= PATCH_SIZE;
	y /= PATCH_SIZE;
	COLLISION* tmp = mask[y*height + x];
	return (x >= 0 && y >= 0 && x < width && y < height && (!tmp || (tmp->pA && tmp->pA->getGroup() == groupIgnore)));
}

bool PATHER::passable_unit(int x, int y, COLLISION* col) {
	x /= PATCH_SIZE;
	y /= PATCH_SIZE;
	COLLISION* tmp = mask[y*height + x];
	// Check if outside world. If an agent watch out for other agents, otherwise just stationaries. If in group formation then don't collide with agents from the same team that are also in formation and not fighting.
	return (x >= 0 && y >= 0 && x < width && y < height && (tmp == col || !tmp || (!col && !tmp->pS) || ((tmp && col && col->pA && tmp->pA) && ((tmp->pA->getGroup() == col->pA->getGroup() && tmp->pA->is_near_dest() && col->pA->is_near_dest()) || (tmp->pA->get_team() == col->pA->get_team() && tmp->pA->is_moving())))));
}

AGENT* PATHER::who_collision(int x, int y)
{
	x /= PATCH_SIZE;
	y /= PATCH_SIZE;
	COLLISION* tmp = mask[y*height + x];
	// Return NULL if outside world, stationary, or empty. Return AGENT* if there is an agent at this location.
	if (x >= 0 && y >= 0 && x < width && y < height && tmp && tmp && tmp->pA) return tmp->pA;
	else return NULL;
}

PATHER::COLLISION_TYPE PATHER::collision_type(int x, int y)
{
	x /= PATCH_SIZE;
	y /= PATCH_SIZE;
	if (x < 0 || y < 0 || x >= width || y >= height) return STATIONARY_COLLIDE;
	COLLISION* tmp = mask[y*height + x];
	if (!tmp) return NO_COLLIDE;
	if (tmp->pA) return AGENT_COLLIDE;
	return STATIONARY_COLLIDE;
}

AGENT* PATHER::check_agent_collision(int x, int y)
{
	x /= PATCH_SIZE;
	y /= PATCH_SIZE;
	if (x < 0 || y < 0 || x >= width || y >= height) return NULL;
	return mask[y*height + x]->pA;
}

STATIONARY* PATHER::check_stationary_collision(int x, int y)
{
	x /= PATCH_SIZE;
	y /= PATCH_SIZE;
	if (x < 0 || y < 0 || x >= width || y >= height) return NULL;
	return mask[y*height + x]->pS;
}

bool PATHER::is_on_map(int x, int y)
{
	return (x >= 0 && y >= 0 && x/PATCH_SIZE < width && y/PATCH_SIZE < height);
}

// clear passibility
void PATHER::clear(int x, int y)
{
	mask[(y/PATCH_SIZE)*height + x/PATCH_SIZE] = NULL;
}

// only clear the passibility if it is caused by this agent
void PATHER::clear(int x, int y, COLLISION* col) {
	COLLISION** tc = &(mask[(y/PATCH_SIZE)*height + x/PATCH_SIZE]);
	if (*tc == col) *tc = NULL;
}

// clear whole mask
void PATHER::clearMask(void) {
	for (int y=0; y<height; y++) {
		for (int x=0; x<width; x++) {
			mask[y*height + x] = NULL;
		}
	}
}

// block off location on mask
void PATHER::set(int x, int y, COLLISION * col) {
	mask[(y/PATCH_SIZE)*height + x/PATCH_SIZE] = col;
}

// block off location on mask
void PATHER::set_autounset(int x, int y, COLLISION * col) {
	mask[(y/PATCH_SIZE)*height + x/PATCH_SIZE] = col;
	if (col) {
	    if (col->x >= 0 && col->y >= 0) clear(col->x, col->y, col);
	    col->x = x;
	    col->y = y;
	}
}

// set a block
void PATHER::setBlock(int x, int y, COLLISION * col) {
    int mx = x/PATCH_SIZE;
    int my = y/PATCH_SIZE;
    COLLISION** ma = &mask[my*height + mx];

	if (my > 0 && *(ma - height) == NULL) *(ma - height) = col;
	if (my < height && *(ma + height) == NULL) *(ma + height) = col;
	if (mx > 0 && *(ma - 1) == NULL) *(ma - 1) = col;
	if (mx < width && *(ma + 1) == NULL) *(ma + 1) = col;

	if (my > 0 && mx > 0 && *(ma - height - 1) == NULL) *(ma - height - 1) = col;
	if (my < height && mx > 0 && *(ma + height - 1) == NULL) *(ma + height) = col;
	if (my > 0 && mx < width && *(ma - height + 1) == NULL) *(ma - height + 1) = col;
	if (my < height && mx < width && *(ma + height + 1) == NULL) *(ma + height + 1) = col;
}

// clear a block
void PATHER::clearBlock(int x, int y, COLLISION * col) {
    int mx = x/PATCH_SIZE;
    int my = y/PATCH_SIZE;
    COLLISION** ma = &mask[my*height + mx];

	if (my > 0 && *(ma - height) == col) *(ma - height) = NULL;
	if (my < height && *(ma + height) == col) *(ma + height) = NULL;
	if (mx > 0 && *(ma - 1) == col) *(ma - 1) = NULL;
	if (mx < width && *(ma + 1) == col) *(ma + 1) = NULL;

	if (my > 0 && mx > 0 && *(ma - height - 1) == col) *(ma - height - 1) = NULL;
	if (my < height && mx > 0 && *(ma + height - 1) == col) *(ma + height) = NULL;
	if (my > 0 && mx < width && *(ma - height + 1) == col) *(ma - height + 1) = NULL;
	if (my < height && mx < width && *(ma + height + 1) == col) *(ma + height + 1) = NULL;
}

// set, clear or check large areas

// set box
void PATHER::setBox(int x1, int y1, int x2, int y2, COLLISION * col) {
    if (!col) col = &generic;
	x1 /= PATCH_SIZE;
	y1 /= PATCH_SIZE;
	x2 /= PATCH_SIZE;
	y2 /= PATCH_SIZE;
	COLLISION** c;
	for (int y=y1; y<y2; y++) {
		c = &mask[y*height];
		for (int x=x1; x<x2; x++) {
			if (!c[x]) c[x] = col;
		}
	}
}

// clear box
void PATHER::clearBox(int x1, int y1, int x2, int y2) {
	x1 /= PATCH_SIZE;
	y1 /= PATCH_SIZE;
	x2 /= PATCH_SIZE;
	y2 /= PATCH_SIZE;
	COLLISION** c;
	for (int y=y1; y<=y2; y++) {
		c = &mask[y*height];
		for (int x=x1; x<=x2; x++) {
			c[x] = NULL;
		}
	}
}

// set circle
void PATHER::setCircle(int x, int y, double r, COLLISION * col) {
	if (!col) col = &generic;
	x /= PATCH_SIZE;
	y /= PATCH_SIZE;
	int tx, ty;
	double s = 2 * M_PI * (double)PATCH_SIZE/(M_PI * r * r);
	r /= PATCH_SIZE;
	for (double t=0; t<=2*M_PI; t+=s) {
		tx = (int)(cos(t) * r) + x;
		ty = (int)(sin(t) * r) + y;
		if (tx < 0 || ty < 0 || tx >= width || ty >= height) continue;
		if (!mask[ty*height+tx]) mask[ty*height+tx] = col;
	}
}

// clear circle
void PATHER::clearCircle(int x, int y, double r) {
	x /= PATCH_SIZE;
	y /= PATCH_SIZE;
	int tx, ty;
	double s = 2 * M_PI * (double)PATCH_SIZE/(M_PI * r * r);
	r /= PATCH_SIZE;
	for (double t=0; t<=2*M_PI; t+=s) {
		tx = (int)(cos(t) * r) + x;
		ty = (int)(sin(t) * r) + y;
		if (tx < 0 || ty < 0 || tx >= width || ty >= height || !mask[ty*height+tx]) continue;
		mask[ty*height+tx] = NULL;
	}
}

// Pathfind (medium) mask

void PATHER::set_box_m(int x1, int y1, int x2, int y2)
{
	x1 = x1/smask.areasize - EXTRA_AREA_STATIONARIES;
	y1 = y1/smask.areasize - EXTRA_AREA_STATIONARIES;
	x2 = x2/smask.areasize + EXTRA_AREA_STATIONARIES;
	y2 = y2/smask.areasize + EXTRA_AREA_STATIONARIES;
	for (int x=x1-BUFFER_AREA_STATIONARIES; x<=x2+BUFFER_AREA_STATIONARIES; x++) {
		for (int y=y1-BUFFER_AREA_STATIONARIES; y<=y2+BUFFER_AREA_STATIONARIES; y++) {
			smask.set_boundscheck(x, y, (x < x1 || x > x2 || y < y1 || y > y2) ? MASK::BAD : MASK::BLOCK);
		}
	}
}

void PATHER::set_stationary_m(int x, int y)
{
	const signed char offsets[8][2] = {{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1},{0,1},{1,1}};
	x /= smask.areasize;
	y /= smask.areasize;
	smask.set_boundscheck(x, y, MASK::BLOCK);
	for (int i=0; i<8; i++) smask.set_boundscheck(x+offsets[i][0], y+offsets[i][1], MASK::BAD);
}

bool PATHER::passable_m(int x, int y)
{
    x /= smask.areasize;
	y /= smask.areasize;
	if (smask.check_boundscheck(x, y) != MASK::BLOCK) return true;
	else return false;
}

bool PATHER::is_open(int x, int y)
{
    x /= smask.areasize;
	y /= smask.areasize;
	if (smask.check_boundscheck(x, y) == MASK::OPEN) return true;
	else return false;
}

void PATHER::make_open(int x, int y)
{
    x /= smask.areasize;
	y /= smask.areasize;

	smask.set_to_boundscheck(x, y, MASK::OPEN);
}

void PATHER::set_blocked(int x, int y)
{
    x /= smask.areasize;
	y /= smask.areasize;

	smask.set_boundscheck(x, y, MASK::BLOCK);
}

void PATHER::set_notblocked(int x, int y)
{
    x /= smask.areasize;
	y /= smask.areasize;

	if (smask.check_boundscheck(x, y) == MASK::BLOCK) smask.set_to_boundscheck(x, y, MASK::OPEN);
}

void PATHER::add_building_m(int x, int y)
{
	const signed char offsets[4][2] = {{1,0},{0,-1},{-1,0},{0,1}};

	// collision squares
	const int b_width = STATIONARY_SIZE/2;
	setBox(x - b_width, y - b_width, x + b_width, y + b_width);

	// pathfind block
	x /= smask.areasize;
	y /= smask.areasize;
	smask.tmp_set(x, y, MASK::REALLY_BAD);
	for (int i=0; i<4; i++) smask.tmp_increase(x+offsets[i][0], y+offsets[i][1]);
}

void PATHER::remove_building_m(int x, int y)
{
	const signed char offsets[4][2] = {{1,0},{0,-1},{-1,0},{0,1}};

	// collision squares
	const int b_width = STATIONARY_SIZE/2;
	clearBox(x - b_width, y - b_width, x + b_width, y + b_width);

	// pathfind clear
	x /= smask.areasize;
	y /= smask.areasize;
	smask.tmp_reset(x, y);
	for (int i=0; i<4; i++) smask.tmp_reset(x+offsets[i][0], y+offsets[i][1]);
}

void PATHER::clearMask_m(void)
{
    smask.clearMask();
}

int PATHER::get_pathfind_areasize(void)
{
    return smask.areasize;
}

// PATHFINDING

TASK* PATHER::get_path(int x, int y, int fx, int fy) {
	return MapSearchNode::findPath(x, y, fx, fy, &smask, &mapsearchnode_error);
}

TASK* PATHER::get_path_closest(int x, int y, int fx, int fy) {
	TASK* t;
	bool replace_next = false;
	if (smask.check_boundscheck(fx/smask.areasize, fy/smask.areasize) == MASK::BLOCK) {
		smask.closest_open(fx/smask.areasize, fy/smask.areasize, fx, fy);
		fx *= smask.areasize;
		fy *= smask.areasize;
		replace_next = true;
	}
	t = MapSearchNode::findPath(x, y, fx, fy, &smask, &mapsearchnode_error, !replace_next);
	if (t) t->replace_next = replace_next;
	return t;
}


void PATHER::group_arrange(int num, int &nx, int &ny)
{
	// cheat to make the first few look good
	static int p[9][2] = {{0,0},{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
	if (num < 9) {
		nx = p[num][0];
		ny = p[num][1];
		return;
	}
	// algorithm for any number of groups
	int r=1;
	if (!num) return;
	while (1) {
		ny = r;
		for (nx=-r; nx < r; nx++) if (!--num) return;
		ny = -r;
		for (nx=r; nx > -r; nx--) if (!--num) return;
		nx = r;
		for (ny=r; ny > -r; ny--) if (!--num) return;
		nx = -r;
		for (ny=-r; ny < r; ny++) if (!--num) return;
		r++;
	}
}
