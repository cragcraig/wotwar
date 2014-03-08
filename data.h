#ifndef DATA_H
#define DATA_H

#include "wotwar_classes.h"
#include "data_types.h"
#include <string>
#include <map>
#include <vector>

class UI_DATA {
    public:
        FONT* lfont;
        BITMAP* build_button;
        BITMAP* highlight_build_button;
        BITMAP* mouse_cursor;
        BITMAP* resource_icon;
        BITMAP* logo;
        UI_DATA();
        ~UI_DATA();
};

class DATA {
	public:
		enum DATA_TYPE {NONE, UNIT, ANIMATION};
		bool isDataOk(void) {return dataOk;}
		UNIT_DATA * getUnitDataPointer(const char * unitName);
		ANIMATION_DATA * getAnimationDataPointer(const char * unitName);
		BUILDING_DATA * getBuildingDataPointer(const char * buildingName);
		WORLD * world;
		const char * get_random_unit(void);

		UI_DATA& ui_data;

		int get_numbcivs(void);
		const char * get_civname(int civ);

		vector<string> building_list;
		vector<string> civs;
		vector<string> unit_list;

		DATA(const char * fileName, WORLD * w, UI_DATA& ui_data);
		~DATA(void);

	private:
		char* getTextLine(char * line);
		int getIntLine(char * line);
		void addUnitData(UNIT_DATA * pUnitData);
		void addAnimationData(ANIMATION_DATA * pAnimationData);
		bool dataOk;
		int num_units;
		UNIT_DATA * firstUnit;
		ANIMATION_DATA * firstAnimation;
		DATA::DATA_TYPE startsDataField(char * line);
        map<string, BUILDING_DATA, cfg_cmp> building_lib;
};

#endif
