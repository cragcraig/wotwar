#ifndef ATTACK_TYPE_H
#define ATTACK_TYPE_H

#include "wotwar_classes.h"

class ATTACK_TYPE
{
    private:
        int field;

    public:
        ATTACK_TYPE(void);
        char particle[128];
        char effect_particle[128];
        int damage;
		int chain_factor;

        bool is_inited(void) { return field; };

        bool set_data(int d);
        bool set_data(const char * s);
};






#endif
