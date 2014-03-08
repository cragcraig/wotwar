#include <math.h>
#include "wotwar.h"

PARTICLE::PARTICLE(PARTICLE_SYSTEM* part_sys, PARTICLE_DATA& data, AGENT_HANDLER * ah, double x, double y, double rotation, int life, const ATTACK_TYPE * attack_t, int target_gid, int target_uid, int source_gid, int chain_factor) : part_sys(part_sys), x(x), y(y), theta(rotation * M_PI / 180), life(data.life), data(data), attack_t(attack_t), target_gid(target_gid), target_uid(target_uid), source_gid(source_gid), frame_index(0.), expansion(0.), sprite(NULL), prev_frameindex(-10), is_subimg(false), done_damage(false), agent_handler(ah), chain_factor(chain_factor)
{
	// setup chaining
	if (chain_factor < 0) {
		if (attack_t) this->chain_factor = attack_t->chain_factor;
		else chain_factor = 0;
	}

	// set life
	if (life > 0) this->life = life;
	start_len = this->life;
    
	if (data.particle_type == 0) this->life -= data.radius/2; // ligntning has double life
	if (data.particle_type == 1) this->life *= 2; // ligntning has double life
	if (data.particle_type == 2) {
		do_damage();
		chain();
	}
    if (data.particle_type == 3) this->life = data.speed; // static particles

	if (data.type == 2) this->life = 1; // play once and die (life doesn't matter)
	if (data.type == 3) this->frame_index = rand() % data.num_frames; // random start frame
    starting_life = this->life;
    if (data.particle_type == 2) this->life = data.speed;
	full_life = this->life;
}

PARTICLE::~PARTICLE()
{
	if (sprite && is_subimg) destroy_bitmap(sprite);
}

void PARTICLE::update(DISPLAY& disp, bool is_draw)
{
    // update
    if (data.particle_type == 3 || data.particle_type == 2) life -= 1;
	else if (data.type != 2) life -= data.speed;

    if (data.particle_type == 0 || (data.particle_type == 1 && life < starting_life/2)) {
        x += data.speed * cos(theta);
        y += data.speed * sin(theta);
    }

    // update frame
    frame_index += data.frame_rate;
    if (frame_index >= data.num_frames) {
        if (data.type == 0) {
            frame_index -= data.num_frames;
        } else if (data.type) {
            frame_index = data.num_frames-1;
            if (data.type == 2) life = 0;
        }
    }

    // update expansion (zoom type)
    if (data.particle_type == 1) {
        if (life < starting_life/2) {
			expansion = life / starting_life;
			if (!done_damage) do_damage();
			if (chain_factor) chain();
		}
        else expansion = (starting_life - life) / starting_life;
    } else if (data.particle_type == 2) {
    	// update expansion (solid type)
		expansion = 1.0;
	}

    // draw
    if (is_draw && is_on_screen(disp)) draw(disp);
}

void PARTICLE::draw(DISPLAY& disp)
{
    int drawX = disp.toScreenX((int) x);
	int drawY = disp.toScreenY((int) y);
	BITMAP * fsprite = data.frames[(int)frame_index];

	// create subsprite
	if ((data.particle_type == 1 || data.particle_type == 2) && start_len < fsprite->w*2) {
		if ((int)frame_index != prev_frameindex) {
			prev_frameindex = (int)frame_index;
			if (sprite && is_subimg) destroy_bitmap(sprite);
			sprite = create_sub_bitmap(fsprite, fsprite->w/2 - start_len/2, 0, (start_len > 20) ? start_len : 20, fsprite->h);
		}
	} else {
		sprite = fsprite;
		is_subimg = false;
	}

	// draw
	if (data.particle_type == 0 || data.particle_type == 3) pivot_sprite(disp.buffer, sprite, drawX, drawY, sprite->w / 2, sprite->h / 2, itofix( (int) (theta / M_PI * 256/2) ));
	else if (data.particle_type == 1 || data.particle_type == 2) pivot_scaled_sprite(disp.buffer, sprite, drawX, drawY, 0, sprite->h / 2, itofix( (int) (theta / M_PI * 256/2) ), ftofix(expansion * (double)starting_life/(double)sprite->w));
}

void PARTICLE::do_damage()
{
	// check when to do damage
	if ((data.particle_type != 0 && dead()) || done_damage) return;

    // do attack stuff
    if (attack_t) {
        GROUP * gt = agent_handler->find_group(target_gid);
        if (gt) {
            AGENT * at = gt->find_unit(target_uid);
            if (at && at->dist_to(x, y) < 4*data.radius + 2*at->getRadius()) {
                GROUP * gs = agent_handler->find_group(source_gid);
                at->hurt_by(attack_t->damage, gs);
				if (attack_t->effect_particle[0]) part_sys->add(attack_t->effect_particle, at->get_x(), at->get_y(), theta, 1, NULL, 0, 0, 0, true);
			}
        }
    }
    done_damage = true;
}

void PARTICLE::chain()
{
	if (!chain_factor) return;

	int sx = x + (start_len - 5) * cos(theta);
	int sy = y + (start_len - 5) * sin(theta);

	if (attack_t) {
        GROUP * gt = agent_handler->find_group(target_gid);
        if (gt && !gt->is_single_unit()) {
            AGENT * at = gt->find_nextunit(target_uid);
            if (at) {
				int xoff = at->get_x() - sx;
				int yoff = at->get_y() - sy;
				part_sys->add(attack_t->particle, sx, sy, atan2(yoff, xoff)/M_PI * 180., sqrt(xoff*xoff + yoff*yoff), attack_t, target_gid, at->get_id(), source_gid, true, chain_factor-1);
			}
        }
	}
	
	chain_factor = 0;
}

bool PARTICLE::is_on_screen(DISPLAY& disp)
{
    if (x + data.radius > disp.getWorldX() && x - data.radius < disp.getWorldX() + disp.getWidth() && y + data.radius > disp.getWorldY() && y - data.radius < disp.getWorldY() + disp.getHeight()) return true;
	else return false;
}

bool PARTICLE::dead(void)
{
    return (life <= 0);
}
