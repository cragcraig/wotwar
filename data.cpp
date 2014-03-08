#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include "wotwar.h"

FONT * game_font = NULL;

DATA::DATA(const char * filepath, WORLD * w, UI_DATA& ui_data) : dataOk(true), firstUnit(NULL), firstAnimation(NULL), world(w), ui_data(ui_data), num_units(0)
{

	// temp. variables
	UNIT_DATA * tmpUnitData = NULL;
	ANIMATION_DATA * tmpAnimationData = NULL;
	BUILDING_DATA * tmpBuildingData = NULL;
	string path(filepath);
	string file;

	// load unit datafile
	file = path + "units.dat";
	CONFIG_FILE df(file.c_str());

	CONFIG_BLOCK * block;

	while (block = df.next()) {
	    tmpUnitData = new UNIT_DATA(block->get_name());
	    // load data
	    while (block->line_type()) {
	        switch (block->line_type()) {
                case CONFIG_LINE::INTEGER :
                    tmpUnitData->setData(block->read_integer());
                break;
                case CONFIG_LINE::STRING :
                    tmpUnitData->setData(block->read_string());
                break;
	        }
	        block->next();
	    }
	    // store
	    if (!tmpUnitData->is_building) unit_list.push_back(string(block->get_name()));
		if (tmpUnitData) { // append new UNIT_DATA to the linked list
            addUnitData(tmpUnitData);
            tmpUnitData = NULL;
        }
	}

	// load animation datafile
	file = path + "animation.dat";
	CONFIG_FILE af(file.c_str());

	while (block = af.next()) {
	    tmpAnimationData = new ANIMATION_DATA(block->get_name());
	    // load data
	    while (block->line_type()) {
	        switch (block->line_type()) {
                case CONFIG_LINE::INTEGER :
                    tmpAnimationData->setData(block->read_integer());
                break;
                case CONFIG_LINE::STRING :
                    tmpAnimationData->setData(block->read_string());
                break;
	        }
	        block->next();
	    }
	    // store
	    if (tmpAnimationData) { // append new ANIMATION_DATA to the linked list
            addAnimationData(tmpAnimationData);
            tmpAnimationData = NULL;
        }
	}

    // load building datafile
    file = path + "buildings.dat";
	CONFIG_FILE bf(file.c_str());
	string tstr;

	while (block = bf.next()) {
	    building_lib[block->get_name()] = BUILDING_DATA();
	    // set name
	    building_lib[block->get_name()].give_dataobj(this);
	    building_lib[block->get_name()].set_data(block->get_name());
	    // set generic buildplot types
        building_lib[block->get_name()].is_generic = !strncmp(block->type.c_str(), "genericplot", 11) ? true : false;
        building_lib[block->get_name()].is_buildsite = !strncmp(block->type.c_str(), "buildplot", 9) ? true : false;
        // add to list
        building_list.push_back(string(block->get_name()));
	    // load data
	    while (block->line_type()) {
	        switch (block->line_type()) {
                case CONFIG_LINE::INTEGER :
                    building_lib[block->get_name()].set_data(block->read_integer());
                break;
                case CONFIG_LINE::STRING :
                    building_lib[block->get_name()].set_data(block->read_string());
                break;
	        }
	        block->next();
	    }
	}

	// load civilizations
	char buf[1024];
	int i;
	file = path + "civilizations.dat";
	FILE* fciv = fopen(file.c_str(), "r");
	if (!fciv) panic("missing civilizations.dat!");
    printf("success: civilizations.dat loaded\n");

    while (!feof(fciv)) {
        while (fgetc(fciv) != '"') if (feof(fciv)) break;
        if (feof(fciv)) break;
        fgets(buf, 1024, fciv);
        i = 0;
        while (buf[i] != '"' && buf[i] != '\n' && i < 1023) i++;
        if (buf[i] == '"') {
            buf[i] = '\0';
            civs.push_back(string(buf));
            printf(" civ: '%s'\n", buf);
        } else {
            panic("unmatched \" in civilizations.dat");
        }
    }

    if (!civs.size()) panic("civilizations.dat doesn't define any factions!");

	fclose(fciv);
}

int DATA::get_numbcivs(void)
{
    return civs.size();
}

const char * DATA::get_civname(int civ)
{
    if (civ < civs.size()) return civs[civ].c_str();
    return NULL;
}

const char * DATA::get_random_unit(void)
{
    if (!num_units) return NULL;
    UNIT_DATA * pTmp = firstUnit;
    int u = rand()%num_units + 1;
    do {
        if (pTmp->maxSpeed) u--;
        if (u <= 0) break;
    } while (pTmp = pTmp->next);

	return pTmp ? pTmp->name : NULL;
}

DATA::DATA_TYPE DATA::startsDataField(char * line)
{
	if (!strncmp(line, "unit", 4)) return DATA::UNIT;
	else if (!strncmp(line, "ani", 3)) return DATA::ANIMATION;
	else return DATA::NONE;
}

DATA::~DATA(void)
{
	// delete all unit data
	UNIT_DATA * pTmp = firstUnit;
	UNIT_DATA * pTmpNext;
	while (pTmp) {
		pTmpNext = pTmp->next;
		delete pTmp;
		pTmp = pTmpNext;
	}
	// delete all animation data
	ANIMATION_DATA * pTmpA = firstAnimation;
	ANIMATION_DATA * pTmpANext;
	while (pTmpA) {
		pTmpANext = pTmpA->next;
		delete pTmpA;
		pTmpA = pTmpANext;
	}
}

char * DATA::getTextLine(char * line)
{
	// find text length
	char * ptr = line;
	char end_type;
	unsigned int length = 0;
	while (*ptr != '\'' && *ptr != '"') ptr++;
	end_type = *ptr;
	ptr++;
	if (ptr - line > 100) dataOk = false; // report corruption
	while (*ptr != end_type) {
		ptr++;
		length++;
	}

	// put text in a C-String
	char * returnString = new char[length + 1];
	ptr = returnString;
	while (*line != end_type) line++;
	line++;
	while (*line != end_type) {
		*ptr++ = *line++;
	}
	*ptr = 0;
	return returnString;
}

int DATA::getIntLine(char * line)
{
	// get text from line
	char * text = getTextLine(line);
	char * ptr = text;
	// convert to an int and clean up extra data
	int returnInt;
	returnInt = 0;
	while (*ptr >= '0' && *ptr <= '9') {
		returnInt = (returnInt * 10) + *ptr++ - '0';
	}
	delete [] text;
	return returnInt;
}

void DATA::addUnitData(UNIT_DATA * pUnitData)
{
	// attach to last unit data instance in the linked list
	UNIT_DATA ** pTmp = &firstUnit;
	while (*pTmp) pTmp = &((*pTmp)->next);
	*pTmp = pUnitData;
	if (pUnitData->maxSpeed) num_units++;
}

void DATA::addAnimationData(ANIMATION_DATA * pAnimationData)
{
	// attach to last unit data instance in the linked list
	ANIMATION_DATA ** pTmp = &firstAnimation;
	while (*pTmp) pTmp = &((*pTmp)->next);
	*pTmp = pAnimationData;
}

// get pointer to unit data
UNIT_DATA * DATA::getUnitDataPointer(const char * unitName)
{
	UNIT_DATA * pTmp = firstUnit;
	while (pTmp && strcmp(pTmp->name, unitName)) pTmp = pTmp->next;
	return pTmp;
}

// get pointer to animation data
ANIMATION_DATA * DATA::getAnimationDataPointer(const char * unitName)
{
	ANIMATION_DATA * pTmp = firstAnimation;
	int count = 0;
	while (pTmp) { // count possibilities
		if (!strcmp(pTmp->name, unitName)) count++;
		pTmp = pTmp->next;
	}
	pTmp = firstAnimation;
	if (!count) {
		printf("no animation data for \"%s\"!\nFatal Error: missing data resulted in division by zero in data.cpp: getAnimationDataPointer()\n", unitName);
		panic("missing data");
	}
	count = (rand() % count) + 1;
	int i = 0;
	while (pTmp) { // choose possibility
		if (!strcmp(pTmp->name, unitName)) i++;
		if (i == count) break;
		else pTmp = pTmp->next;
	}
	return pTmp;
}

BUILDING_DATA * DATA::getBuildingDataPointer(const char * buildingName)
{
    BUILDING_DATA * build_dat = NULL;
    if (building_lib.find(buildingName) != building_lib.end()) build_dat = &building_lib.find(buildingName)->second;

    return build_dat;
}


// UI_DATA

#define UI_DATA_UNLOAD(bmp) {if (bmp) { destroy_bitmap(bmp); bmp = NULL; }}
#define UI_DATA_LOAD(img, file) {img = load_bitmap(file, NULL); if (!img) {printf("UI_DATA: missing %s\n", file); panic("unable to find a UI bitmap, check log");}}

UI_DATA::UI_DATA() : build_button(NULL)
{
    // load building UI button data
    build_button = load_bitmap("data/gui/button.bmp", NULL);
    highlight_build_button = load_bitmap("data/gui/buttonh.bmp", NULL);
    if (!build_button || build_button->w != POPUP_SIZE || build_button->h != POPUP_SIZE) {
        printf("BUILDING_DATA: missing or small gui/button.bmp (%d x %dpx)\n", POPUP_SIZE, POPUP_SIZE);
        panic("bad/missing building UI chrome");
    }
    if (!highlight_build_button || highlight_build_button->w != build_button->w || highlight_build_button->h != build_button->h) {
        printf("BUILDING_DATA: missing or unmatched size gui/buttonh.bmp (%d x %dpx)\n", POPUP_SIZE, POPUP_SIZE);
        panic("bad/missing building UI chrome");
    }

    // generic image loads
    UI_DATA_LOAD(mouse_cursor, "data/gui/cursor.bmp");
    UI_DATA_LOAD(resource_icon, "data/gui/resource_icon.bmp");
    UI_DATA_LOAD(logo, "data/gui/logo.bmp");

    // load font
	lfont = load_font("data/gui/font.bmp", NULL, NULL);
	if (!lfont) panic("missing the font for the gui (gui/font.bmp)");
	game_font = lfont;
	font = lfont; // for allegro gui
}

UI_DATA::~UI_DATA()
{
    UI_DATA_UNLOAD(build_button);
    UI_DATA_UNLOAD(highlight_build_button);
    UI_DATA_UNLOAD(mouse_cursor);
    UI_DATA_UNLOAD(resource_icon);
    UI_DATA_UNLOAD(logo);

    // release font
	if (lfont) destroy_font(lfont);
}
