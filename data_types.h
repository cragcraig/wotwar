#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <cstring>
#include "wotwar_classes.h"

class UNIT_DATA {
	public:
		char name[255];
		UNIT_DATA * next;
		int field;

		// UNIT DATA
		int maxHealth, radius, rattack_frame, range, r_horiz_offset;
		double maxSpeed, minSpeed, acceleration, baseRotateRate, quickRotateRate, avgRotateRate, maxTurnRadius, damage;
		bool melee, ranged, killable, is_building;
		char ranged_attack[128];
		char death_particle[128];
		char blood_particle[128];
		// END DATA

		bool setData(const char * s);
		bool setData(int d);

		UNIT_DATA(const char * name) : field(0), next(NULL), maxSpeed(0), minSpeed(0), acceleration(0), baseRotateRate(0), quickRotateRate(0), avgRotateRate(0), maxTurnRadius(0), melee(false), ranged(false), damage(0.), killable(true), is_building(true)
		{
			ranged_attack[0] = '\0';
			death_particle[0] = '\0';
			blood_particle[0] = '\0';

			strcpy(this->name, name);
			printf(" unit: '%s'\n", name);
			ranged_attack[0] = '\0';
		}

		void calcMaxTurnRadius(void)
		{
			maxTurnRadius = maxSpeed * sin((180.0 - baseRotateRate) / 2.0 * M_PI / 180.0) / sin(baseRotateRate * M_PI / 180.0);
			avgRotateRate = baseRotateRate + (quickRotateRate - baseRotateRate) / 2;
		}
};

class FRAME {
	public:
		BITMAP* img;
		FRAME* pNext;
		FRAME(BITMAP* img, int w, int num) {
			this->img = create_bitmap(w, img->h);
			blit(img, this->img, w * num, 0, 0, 0, this->img->w, this->img->h);
			pNext = NULL;
		}
		~FRAME(void) {
			destroy_bitmap(img);
		}
};

class ANIMATION {
	public:
		float speed;
		int length;
		int width;
		BITMAP* img;
		FRAME* pFirst;

		ANIMATION(char * fileName) {
			speed = 1.0;
			pFirst = NULL;
			img = load_bitmap(fileName, NULL);
		}

		void setup(void)
		{ // generate frames
			FRAME ** tmpFrame = &pFirst;
			if (!img) {
				printf("fatal error: ANIMATION sequence is defined but the image cannot be found\n");
				panic("missing bitmap");
			}
			for (int i = 0; i < length; i++) {
				*tmpFrame = new FRAME(img, width, i);
				tmpFrame = &((*tmpFrame)->pNext);
			}
			destroy_bitmap(img);
			img = NULL;
		}

		~ANIMATION(void)
		{
			FRAME * pTmp = pFirst;
			FRAME * pTmpNext;
			while (pTmp) {
				pTmpNext = pTmp->pNext;
				delete pTmp;
				pTmp = pTmpNext;
			}
		}
};

#define NUM_ANIMATIONS 10
class ANIMATION_DATA {
	public:
		char name[255];
		ANIMATION_DATA * next;
		int field, width;

		enum ANI_TYPE {STAND, IDLE, MOVE, ATTACK, RANGED, DIE};

		ANIMATION * animations[NUM_ANIMATIONS];

		void setData(const char * baseFileName);
		void setData(int speed);

		ANIMATION * getAnimation(ANIMATION_DATA::ANI_TYPE t);

		ANIMATION_DATA(const char * name)
		{
			next = NULL;
			strcpy(this->name, name);
			field = 0;
		}

		~ANIMATION_DATA(void)
		{
			for (int i = 0; i < NUM_ANIMATIONS; i++) {
				if (animations[i]) delete animations[i];
			}
		}
};

#endif
