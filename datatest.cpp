#include <cstddef>
#include <allegro.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include "data.h"

using namespace std;

void init(void) {
	allegro_init();
	install_keyboard();
	install_timer();
	install_mouse();
	install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT,0);

	set_color_depth(32);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 800, 800, 0, 0);
	set_trans_blender(200,200,200,200); // Use for transparency
}

int main(void)
{	
	init();
	clear_to_color(screen, makecol(255,255,255));
	DATA data("data/units.dat");
	ANIMATION_DATA * knight = data.getAnimationDataPointer("Knight");
	ANIMATION * tmp;
	FRAME * tmpFrame;
	char * f = "test.bmp";
	int j = 0;
	int k = 0;/*
	for (int i = 0; i < NUM_ANIMATIONS; i++) {
		if (tmp = knight->animations[i]) {
			cout << endl << "data exists for animation: " << i << endl;
			tmpFrame = tmp->pFirst;
			j = 0;
			while (tmpFrame) {
				cout << "frame " << j << " | w:" << tmpFrame->img->w << " h:" << tmpFrame->img->h << endl;
				save_bitmap(f, tmpFrame->img, NULL);
				while (!key[KEY_ESC]) ;
				while (key[KEY_ESC]) ;
				j++;
				tmpFrame = tmpFrame->pNext;
			}
		}
	}*/
	
	if (tmp = knight->getAnimation(ANIMATION_DATA::STAND)) {
		cout << endl << "data exists for animation" << endl;
		tmpFrame = tmp->pFirst;
		j = 0;
		while (tmpFrame) {
			cout << "frame " << j << " | w:" << tmpFrame->img->w << " h:" << tmpFrame->img->h << endl;
			save_bitmap(f, tmpFrame->img, NULL);
			while (!key[KEY_ESC]) ;
			while (key[KEY_ESC]) ;
			j++;
			tmpFrame = tmpFrame->pNext;
		}
	}

	return 0;
}
END_OF_MAIN();
