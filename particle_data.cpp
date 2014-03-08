#include "wotwar.h"

PARTICLE_DATA::PARTICLE_DATA(void) : sprite(NULL), field(0), life(0), type(0), frame_rate(0)
{

}

PARTICLE_DATA::PARTICLE_DATA(const char * sprite_path, int life, double speed) : life(life), speed(speed), field(0), type(0), frame_rate(0), particle_type(0)
{
    set_sprite(sprite_path);
}

PARTICLE_DATA::~PARTICLE_DATA(void)
{
    if (frames.size()) {
        for (int i=0; i<num_frames; i++)
            if (frames[i]) destroy_bitmap(frames[i]);
    }
    if (sprite) destroy_bitmap(sprite);
}

void PARTICLE_DATA::set_sprite(const char * s)
{
    if (sprite) return;
    sprite = load_bitmap(s, NULL);
    if (!sprite) {
        printf("PARTICLE_DATA: bitmap %s does not exist\n", s);
        panic("missing particle sprite");
    }
}

bool PARTICLE_DATA::set_data(int d) {

    int tmp, i;

	switch(field) {
		case 1 : speed = d;
			break;
        case 2 :
            particle_type = d;
            break;
        case 3 :
            num_frames = d > 0 ? d : sprite->w/sprite->h;
            tmp = sprite->w/num_frames;
            frames.reserve(num_frames);
            for (i=0; i<num_frames; i++) {
                frames.push_back(create_sub_bitmap(sprite, i*tmp, 0, tmp, sprite->h));
            }
            radius = max(tmp, sprite->h);
			break;
        case 4 : frame_rate = d * 1.0/GOAL_FPS;
			break;
        case 5 : type = d;
            break;
		default: puts("warning: bad particle_data config block");
            return false;
			break;
	}
	field++;
	return true;
}

bool PARTICLE_DATA::set_data(const char * s) {
	switch(field) {
		case 0 : set_sprite(s);
			break;
		default: puts("warning: bad particle_data config block");
            return false;
			break;
	}
	field++;
	return true;
}
