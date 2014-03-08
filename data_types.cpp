#include "data.h"
#include <cstdio>
#include <climits>

// Place data in a UNIT_DATA instance
bool UNIT_DATA::setData(const char * s) {

    // skip other fields for ranged buildings
    if (field == 2) field = 9;

	switch(field) {
		case 7 : // set blood particle
			strcpy(blood_particle, s);
			break;
		case 8 : // set death particle
			strcpy(death_particle, s);
			break;
	    case 9 : // determine attack type
            if (!strcmp(s, "melee")) melee = true, ranged = false;
            else if (!strcmp(s, "ranged")) melee = false, ranged = true;
            else if (!strcmp(s, "both")) melee = true, ranged = true;
            else if (!strcmp(s, "none")) melee = false, ranged = false;
            else printf("warning: unrecognized unit attack type '%s'\n", s);
            // if not melee, skip damage line
            if (!melee && !ranged) field+=5;
            else if (!melee) field++;
            break;
	    case 11 : strcpy(ranged_attack, s); // get name of ranged attack
			break;
		default: puts("warning: bad unit data block");
            return false;
			break;
	}
	field++;
	return true;
}

bool UNIT_DATA::setData(int d) {
	switch(field) {
		case 0 : maxHealth = d;
                if (!maxHealth) {
                    maxHealth = 1000;
                    killable = false;
                }
			break;
		case 1 : radius = d;
			break;
		case 2 : minSpeed = (double)d / 100.;
			break;
		case 3 : 
			is_building = false;
			maxSpeed = (double)d / 100.;
			break;
		case 4 : acceleration = (double)d / 100.;
			break;
		case 5 : baseRotateRate = (double)d / 10.;
			break;
		case 6 : quickRotateRate = (double)d / 10.;
				calcMaxTurnRadius();
			break;
        case 10 : damage = d / (double)GOAL_FPS;
                // if not ranged, skip the ranged config lines
                if (!ranged) field+=4;
            break;
        case 12 : range = d;
            break;
        case 13 : rattack_frame = d;
            break;
        case 14 : r_horiz_offset = d;
            break;
		default: puts("warning: bad unit data block");
            return false;
			break;
	}
	field++;
	return true;
}

// Place data in ANIMATION_DATA instance
void ANIMATION_DATA::setData(const char * baseFileName) {
	if (field) return;
	static const char * fileName[] = {"stand","idle","idle1","move","attack","attack1","attack2","ranged","die","die1"}; // list of possible animations, animation data config file must follow this order
	FILE* exists;
	printf(" searching: \"%s/\"\n", baseFileName);
	for (int i = 0; i < NUM_ANIMATIONS; i++) {
		char fullFileName[100]; // create file name
		sprintf(fullFileName, "%s/%s.bmp", baseFileName, fileName[i]); // generate full filename
		char * tmpPointer;
		while (tmpPointer = strrchr(fullFileName, ' ')) *tmpPointer = '_'; // replace ' ' with '_'
		exists = fopen(fullFileName, "rb"); // check if file exists and create animation object if it does
		if (exists) {
			fclose(exists);
			animations[i] = new ANIMATION(fullFileName);
			printf("  found: \"%s.bmp\"\n", fileName[i]);
		}
		else animations[i] = NULL;
	}
}

void ANIMATION_DATA::setData(int num) {
    // die if there is no stand animation
    if (!animations[0]) panic("missing a 'stand' animation!");

    // get width
    if (!field) {
        width = (num > 0) ? num : animations[0]->img->h;
    } else { // generate animations
        // skip over undefined animations
        while (field-1 < NUM_ANIMATIONS && !animations[field-1]) field++;
        // set data for animation
        int tfield = field-1;
        if (tfield < NUM_ANIMATIONS && animations[tfield]) {
            animations[tfield]->speed = 1.0 / (float)num; // set animation speed
            animations[tfield]->length = animations[tfield]->img->w / width; // set animation length
            animations[tfield]->width = width; // set frame width
            animations[tfield]->setup(); // generate frames
        }
    }
    field++;
}

ANIMATION * ANIMATION_DATA::getAnimation(ANIMATION_DATA::ANI_TYPE t) {
	int base, number;
	switch (t) { // relate types to array locations
		case ANIMATION_DATA::STAND : base = 0; number = 1; break;
		case ANIMATION_DATA::IDLE : base = 1; number = 2; break;
		case ANIMATION_DATA::MOVE : base = 3; number = 1; break;
		case ANIMATION_DATA::ATTACK : base = 4; number = 3; break;
		case ANIMATION_DATA::RANGED : base = 7; number = 1; break;
		case ANIMATION_DATA::DIE : base = 8; number = 2; break;
	}
	int i;
	for (i = 0; i < number; i++) { // choose a random one
		if (!animations[base + i]) break;
	}
	if (!i) { // make sure there is an animation of this type, if not substitute the stand animation
		i = 1;
		base = 0;
	}
	i = rand() % i;
	// return animation, but if anything is wrong just return the fail-proof stand animation
	return ((animations[base + i] && animations[base + i]->pFirst) ? animations[base + i] : animations[0]);
}
