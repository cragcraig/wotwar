#ifndef WOTWAR_H
#define WOTWAR_H

#include "wotwar_classes.h"

#include "agent.h"
#include "agent_handler.h"
#include "config.h"
#include "data_types.h"
#include "data.h"
#include "display.h"
#include "game.h"
#include "group.h"
#include "mapsearchnode.h"
#include "mask.h"
#include "move.h"
#include "sprite.h"
#include "pather_extras.h"
#include "pather.h"
#include "player.h"
#include "task.h"
#include "tasklist.h"
#include "world_extras.h"
#include "world.h"
#include "stationary.h"
#include "screen.h"
#include "particle_data.h"
#include "particle_system.h"
#include "particle.h"
#include "attack_data.h"
#include "attack_type.h"
#include "config_loader.h"
#include "building_data.h"
#include "popup.h"
#include "gui.h"
#include "network.h"
#include "connection.h"

extern FONT * game_font;
void clear_speedcounter(void);
int get_speedcounter(void);
int get_fpsdebt(void);
void strip_nonprintables(char * str);
void exit_game(void);

#endif
