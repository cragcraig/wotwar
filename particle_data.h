#ifndef PARTICLE_DATA_H
#define PARTICLE_DATA_H

#include "wotwar_classes.h"

class PARTICLE_DATA
{
    private:
        int field;
        void set_sprite(const char * s);

    public:
        PARTICLE_DATA(void);
        PARTICLE_DATA(const char * sprite_path, int life, double speed);
        ~PARTICLE_DATA(void);
        BITMAP* sprite;
        int life, radius, num_frames, type;
        int particle_type;
        double frame_rate;
        double speed;
        vector<BITMAP*> frames;

        bool set_data(int d);
        bool set_data(const char * s);
};






#endif
