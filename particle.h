#ifndef PARTICLE_H
#define PARTICLE_H

#include "wotwar_classes.h"

class PARTICLE
{
    private:
        double x, y, theta, frame_index;
        double life, starting_life, full_life;
        const PARTICLE_DATA& data;
        void draw(DISPLAY& disp);
        PARTICLE_SYSTEM* part_sys;
        double expansion;
		BITMAP *sprite;
		int prev_frameindex;
		bool is_subimg;
		int start_len;
		bool done_damage;
		int chain_factor;
		AGENT_HANDLER * agent_handler;

        int target_gid, target_uid, source_gid;

    public:
        const ATTACK_TYPE * attack_t;

        PARTICLE(PARTICLE_SYSTEM* part_sys, PARTICLE_DATA& data, AGENT_HANDLER * ah, double x, double y, double rotation, int life=-1, const ATTACK_TYPE * attack_t = NULL, int target_gid=0, int target_uid=0, int source_gid=0, int chain_factor=-1);

        ~PARTICLE();
		void update(DISPLAY& disp, bool is_draw=true);
        void do_damage();
        void chain();
		bool is_on_screen(DISPLAY& disp);
        bool dead(void);
};


#endif

