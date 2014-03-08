#include "wotwar.h"

PARTICLE_SYSTEM::PARTICLE_SYSTEM(const char * filepath, GAME * game) : game(game), agent_handler(NULL)
{
    load_psys(filepath);
}

void PARTICLE_SYSTEM::update_and_draw(DISPLAY& disp, bool draw, bool above)
{
	// choose which particle set
	list<PARTICLE>& particles = above ? particles_above : particles_below;
    
	// get agent handler pointer
    if (!agent_handler) agent_handler = game->get_agent_handler();
    if (!agent_handler) panic("programming error: PARTICLE_SYSTEM::update_and_draw() is unable to connect to agent_handler");

    // update particles
    list<PARTICLE>::iterator p;
    for (p = particles.begin(); p != particles.end(); p++) {
        // update and draw
        p->update(disp, draw);

        // kill dead particle
        if (p->dead()) {
            p->do_damage();
            // don't add anything new after this line -- the particle might no longer exist!
            particles.erase(p--);
        }
    }
}


void PARTICLE_SYSTEM::add(const char * type, double x, double y, double rotation, int life, const ATTACK_TYPE * attack_t, int target_gid, int target_uid, int source_gid, bool above, int chain_factor)
{
	if (!agent_handler) return;

	// choose which particle set
	list<PARTICLE>& particles = above ? particles_above : particles_below;

    // create particle
    PARTICLE_DATA& part_dat = part_lib[type];
    particles.push_back(PARTICLE(this, part_dat, agent_handler, x, y, rotation, life, attack_t, target_gid, target_uid, source_gid, chain_factor));

    if (!part_dat.sprite) { // error, particle type does not exist or was not fully initialized
        printf("error: particle type '%s' is not defined or is missing sprite\n", type);
        panic("attempted to create particle of an undefined type");
    }
}

void PARTICLE_SYSTEM::load_psys(const char * filepath)
{
    string path(filepath);
    path += "particles.dat";
    CONFIG_FILE df(path.c_str());

    CONFIG_BLOCK * block;

    while (block = df.next()) {
        part_lib[block->get_name()] = PARTICLE_DATA();
        printf(" particle: '%s'\n", block->get_name());
        // load data
        while (block->line_type()) {
            switch (block->line_type()) {
                case CONFIG_LINE::INTEGER :
                    part_lib[block->get_name()].set_data(block->read_integer());
                break;
                case CONFIG_LINE::STRING :
                    part_lib[block->get_name()].set_data(block->read_string());
                break;
            }
            block->next();
        }
    }
}
