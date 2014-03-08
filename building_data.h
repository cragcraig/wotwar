#ifndef BUILDING_DATA_H
#define BUILDING_DATA_H

#include "wotwar_classes.h"
#include <vector>

class BUILDABLE
{
    public:
        char name[256];
        BITMAP * sprite;
        bool is_building;
        int quantity, cost, train_time;
};

class BUILDING_DATA
{
    private:
        int field;
        void set_sprite(const char * s);
        DATA * data;

    public:
        BUILDING_DATA(void);
        ~BUILDING_DATA(void);
        BITMAP* sprite;
        int loading_flag;
        int resources_per_sec;
        bool is_generic, is_buildsite;
        char name[256];
        vector<BUILDABLE> buildables;

        BITMAP * generate_sprite_buildable(const char * type);
        void give_dataobj(DATA * data);
        bool set_data(int d);
        bool set_data(const char * s);
};






#endif
