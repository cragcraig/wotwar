#ifndef CONFIG_H
#define CONFIG_H

#include "wotwar_classes.h"

bool skipline(char* line);

// Global config functions
void init_config(GLOBAL_CONFIG& c);
void load_config(const char* filename, GLOBAL_CONFIG& c);

#endif
