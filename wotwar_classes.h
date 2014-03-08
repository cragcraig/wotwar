#ifndef WOTWAR_CLASSES_H
#define WOTWAR_CLASSES_H

#include <boost/cstdint.hpp>
#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <climits>
#include <list>
#include <vector>
#include <allegro.h>

using namespace std;

#include "sizes.h"
#include "keys.h"
#include "link.h"
#include "vec.h"
#include "message.h"

class AGENT;
class AGENT_HANDLER;
class DATA;
class UNIT_DATA;
class FRAME;
class ANIMATION;
class ANIMATION_DATA;
class DISPLAY;
class GAME;
class GROUP;
class MapSearchNode;
class MASK;
class MOVE;
class PATHER;
class COLLISION;
class PLAYER;
class TASKLIST;
class WORLD;
class TILE;
class COORDINATE;
class STATIONARY;
class SCREEN;
class SPRITE;
class PARTICLE_SYSTEM;
class PARTICLE;
class PARTICLE_DATA;
class ATTACK_DATA;
class ATTACK_TYPE;
class BUILDING_DATA;
class BUILDABLE;
class POPUP;
class GUI;
class NETWORK;
class CONNECTION;
class DISPLAY_MSG;

#ifndef __DBL_MAX__ // supply max double value if it is not supplied (too lazy to look up if this is a standard)
#include <limits>
#define __DBL_MAX__ (numeric_limits<double>::max())
#endif

void panic(const char* s);
void global_popup(const char* s);

#endif
