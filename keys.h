#ifndef KEYS_H
#define KEYS_H

#include <allegro.h>

#define KEYS_ADD_TO_SELECTED (key[KEY_LSHIFT] || key[KEY_RSHIFT])
#define KEYS_SHOW_PATHS (key[KEY_LCONTROL] || key[KEY_RCONTROL])
#define KEYS_QUIT (key[KEY_ESC] || key[KEY_Q])

#endif
