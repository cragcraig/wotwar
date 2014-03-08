#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <map>
#include <list>
#include <cstring>
#include <string>
#include "wotwar_classes.h"

class PARTICLE_SYSTEM
{
    private:
        void load_psys(const char * filepath);
        map<string, PARTICLE_DATA, cfg_cmp> part_lib;
        list<PARTICLE> particles_above;
        list<PARTICLE> particles_below;

        GAME * game;
        AGENT_HANDLER * agent_handler;

    public:
        PARTICLE_SYSTEM(const char * filepath, GAME * game);
        void update_and_draw(DISPLAY& disp, bool draw=true, bool above=true);
        void add(const char * type, double x, double y, double rotation, int life=-1, const ATTACK_TYPE * attack_t = NULL, int target_gid=0, int target_uid=0, int source_gid=0, bool above=true, int chain_factor=-1);

};


#endif
