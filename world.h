#ifndef WORLD_H
#define WORLD_H

#include "wotwar_classes.h"
#include <string>
#include <cstdio>

#define NUM_FADERS 16

class WORLD // handles data for (and renders) the world map
{
	private:
		TILE** tile; // array of world tiles
		vector<TILE> tileList; // loaded tiles
		vector<TILE*> curTiles;
		STATIONARY** stationary; // array of world objects (trees, etc.)
		int width, height, tileSize;
		vector<STATIONARY> stationaryList; // loaded world objects
		BITMAP* fader[NUM_FADERS];
		PATHER* pather;
		int stationaryMaxWidth;
		double miniMapScale;

		void loadTiles(const char* filename);
		void loadFaders(void);
		void loadStationaries(const char* filename);
		TILE* getTile(int x, int y);
		void drawTile(BITMAP* buffer, int x, int y, int destX, int destY, int offsetX, int offsetY);
	public:
		void render(BITMAP* bitmap, int destX, int destY, int width, int height, int worldX, int worldY);
		void renderStationary(BITMAP* bitmap, list<COORDINATE>* st, int destX, int destY, int width, int height, int worldX, int worldY);
		void getStationary(list<COORDINATE>* st, int width, int height, int worldX, int worldY);
		void renderMiniMap(BITMAP* bitmap);
		double getMiniMapScale(void);
		int getWidth(void);
		int getHeight(void);
		int getMapWidth(void);
		int getMapHeight(void);
		int getTileSize(void);
		PATHER* getPather(void);

		void save_tiles(FILE * fout);
		void save_stationaries(FILE * fout);
		void load_tiles(FILE * fin);
		void load_stationaries(FILE * fin);

		void setTile(int x, int y, const char* tileName);
		void tile_fill(void);
		void placeObj(int x, int y, const char* type);
		void setCurTile(const char* type);
		void setToCurTile(int x, int y, bool ignore_passibilty = false);
		bool setPassability(int x, int y);

        vector<string> tile_types;
        vector<string> stationary_types;
		void generateForest(int x, int y, int sizefactor, int branchfactor, int expansionfactor, const char* type);
		bool editorClearObj(int x, int y);

		WORLD(int w, int h);
		virtual ~WORLD(void);
};

#endif
