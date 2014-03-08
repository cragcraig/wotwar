#include "wotwar.h"

ATTACK_TYPE::ATTACK_TYPE(void) : field(0), chain_factor(0)
{
    particle[0] = '\0';
    effect_particle[0] = '\0';
}

bool ATTACK_TYPE::set_data(int d) {
	switch(field) {
		case 1 : damage = d;
			break;
		case 2 : chain_factor = d;
			break;
		default: puts("warning: bad attack config block");
            return false;
			break;
	}
	field++;
	return true;
}

bool ATTACK_TYPE::set_data(const char * s) {
	switch(field) {
		case 0 : strcpy(particle, s);
			break;
		case 3 : strcpy(effect_particle, s);
			break;
		default: puts("warning: bad attack config block");
            return false;
			break;
	}
	field++;
	return true;
}

