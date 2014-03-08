#ifndef ATTACK_DATA_H
#define ATTACK_DATA_H

#include <string>
#include "wotwar_classes.h"

class ATTACK_DATA
{
    private:
        void load_attacks(const char * filepath);
        map<string, ATTACK_TYPE, cfg_cmp> attack_lib;

    public:
        ATTACK_DATA(const char * filepath);
        const ATTACK_TYPE * get_type(const char * type);
};

#endif
