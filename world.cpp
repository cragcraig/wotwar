#include "wotwar.h"

// external CONFIG structure, declared in main.cpp and defined in sizes.h
extern GLOBAL_CONFIG global_config;

// constructor
WORLD::WORLD(int w, int h) {
	tileSize = TILE_SIZE;
	loadTiles("data/tiles.dat");
	loadStationaries("data/world.dat");

	// Set size (ensures that it is a multiple of TILES_PER_STATIONARY)
	this->width = (int)ceil((double)w / TILES_PER_STATIONARY)*TILES_PER_STATIONARY;
	this->height = (int)ceil((double)h / TILES_PER_STATIONARY)*TILES_PER_STATIONARY;

	// create pather
	pather = new PATHER(this);

	// Generate Tiles
	tile = new TILE*[width * height];
	stationary = new STATIONARY*[width * height];
	setCurTile(tileList.front().type);
	tile_fill();
	loadFaders();
}

// destructor
WORLD::~WORLD(void) {
	delete [] tile;
	delete [] stationary;
	delete pather;

	// destroy faders
	for (int i=0; i<NUM_FADERS; i++) {
		destroy_bitmap(fader[i]);
	}
}

////////////////////////     RANDOM STUFF     /////////////////////////////////////////////////////////////////////

PATHER* WORLD::getPather(void) {
	return pather;
}

// return pointer to tile struct associated with this location
TILE* WORLD::getTile(int x, int y) {
	if (x < 0 || x >= width || y < 0 || y >= height) return NULL;
	else return tile[x * height + y];
}

// fill entire map with the current tile type
void WORLD::tile_fill(void)
{
    pather->clearAll();

    for (int x=0; x<width; x++) {
		for (int y=0; y<height; y++) {
		    tile[x * height + y] = NULL;
			setToCurTile(x, y, true);
			stationary[x*height + y] = NULL; // clear stationaries
		}
	}
}

// generate a forest
void WORLD::generateForest(int x, int y, int sizefactor, int branchfactor, int expansionfactor, const char* type) {
	char i;
	int j = sizefactor/2;
	bool tooClose;
	const signed char offsets[9][2] = {{0,0},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1},{0,1},{1,1}};
	do {
		if (branchfactor && !(rand()%branchfactor) && sizefactor > 2) generateForest(x, y, sizefactor * 3 / 4, branchfactor, expansionfactor, type);
		else if (getTile(x + TILES_PER_STATIONARY/2, y + TILES_PER_STATIONARY/2)) {
			tooClose = false;
			//for (int j=0; j<9; j++) if (getTile(x + offsets[j][0],y + offsets[j][1]) && stationary[(x + offsets[j][0]) * height + y + offsets[j][1]]) tooClose = true; // check nearby squares for trees
			if (!tooClose) placeObj(x + TILES_PER_STATIONARY/2, y + TILES_PER_STATIONARY/2, type); // place tree if there are no trees surrounding it
		}
		i = rand()%8;
		x += (rand()%(expansionfactor*2 + 1) - expansionfactor) * TILES_PER_STATIONARY;
		y += (rand()%(expansionfactor*2 + 1) - expansionfactor) * TILES_PER_STATIONARY;
		if (j) j--;
	} while (j || rand()%sizefactor);
}

// place a random stationary object of a certain type at (x,y)
void WORLD::placeObj(int x, int y, const char* type) {
	int s,i;
	if (!pather->passable_m(x*tileSize, y*tileSize) || (x < 0 || y < 0 || y >= height || x >= width)) return;
	for (i=0; i<stationaryList.size(); i++) {
		if (!strcmp(type, stationaryList[i].type)) {
			s=i;
			while (i < stationaryList.size() && !strcmp(type, stationaryList[i].type)) i++; // find all of type
			this->stationary[x * height + y] = &stationaryList[rand()%(i-s)]; // add random one to world
			this->stationary[x * height + y]->addCollisions(x*tileSize, y*tileSize, pather); // add to world collision mask
			i = stationaryList.size();
		}
	}
}

bool WORLD::editorClearObj(int x, int y)
{
    if (x < 0 || y < 0 || y >= height || x >= width) return false;
    if (stationary[x * height + y]) {
        stationary[x * height + y] = NULL;
        pather->make_open(x*tileSize, y*tileSize);
        return true;
    }
    return false;
}

// set current tile list to all tiles of type "type"
void WORLD::setCurTile(const char* type) {
	curTiles.clear();
	for (int i=0; i<tileList.size(); i++) {
		if (!strcmp(type, tileList[i].type)) {
			curTiles.push_back(&tileList[i]);
		}
	}
}

// set tile to a random tile from the current tile list
void WORLD::setToCurTile(int x, int y, bool ignore_passibility) {
    TILE* new_tile = curTiles[rand()%curTiles.size()];
    if ((x < 0 || y < 0 || y >= height || x >= width) || (tile[x * height + y] && !strcmp(tile[x * height + y]->type, new_tile->type))) return;
	tile[x * height + y] = new_tile;
	if (!ignore_passibility) setPassability(x, y);
}

// set tile to "tileName"
void WORLD::setTile(int x, int y, const char* tileName) {
    if (x < 0 || y < 0 || y >= height || x >= width) return;
	for (int i=0; i<tileList.size(); i++) {
		if (!strcmp(tileName, tileList[i].name)) {
			tile[x * height + y] = &tileList[i];
			setPassability(x, y);
			return;
		}
	}
}

bool WORLD::setPassability(int x, int y)
{
    int xt = x*tileSize / SMALL_AREA_SIZE * TILES_PER_SMALLAREA;
    int yt = y*tileSize / SMALL_AREA_SIZE * TILES_PER_SMALLAREA;

    // set collisions
    if (tile[x * height + y]->blocked) pather->setBox(x * TILE_SIZE, y * TILE_SIZE, (x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE);
    else pather->clearBox(x * TILE_SIZE, y * TILE_SIZE, (x + 1) * TILE_SIZE - 1, (y + 1) * TILE_SIZE - 1);

    // set pathfind
    if (tile[x * height + y]->blocked) {
        pather->set_stationary_m(x * TILE_SIZE, y * TILE_SIZE);
        return true;
    } else if (!tile[xt * height + yt]->blocked && (xt + 1 >= width || !tile[(xt + 1) * height + yt]->blocked) && (yt + 1 >= height || !tile[xt * height + (yt + 1)]->blocked) && (xt + 1 >= width || yt + 1 >= height || !tile[(xt + 1) * height + (yt + 1)]->blocked)) {
        pather->make_open(x * TILE_SIZE, y * TILE_SIZE);
        return false;
    }

    return false;
}

// get world height in px
int WORLD::getHeight(void) {
	return height * tileSize;
}

// get world width in px
int WORLD::getWidth(void) {
	return width * tileSize;
}

int WORLD::getMapHeight(void) {
	return height;
}

// get world width in px
int WORLD::getMapWidth(void) {
	return width;
}

// get the size of a tile
int WORLD::getTileSize(void) {
	return tileSize;
}

// get Mini-Map scale
double WORLD::getMiniMapScale(void) {
	return miniMapScale;
}

////////////////////////     LOADING     /////////////////////////////////////////////////////////////////////

// loads the faders used by faded_blit
// only call once, it doesn't delete old data
void WORLD::loadFaders(void) {
	char filename[30];
	int die = false;
	printf("Loading %d faders: \"data/alpha/tri0..tri%d.bmp\"\n", NUM_FADERS, NUM_FADERS-1);
	for (int i=0;i<NUM_FADERS;i++) {
		sprintf(filename, "data/alpha/tri%d.bmp", i);
		if (!(fader[i] = load_bitmap(filename, NULL))) {
			die = true;
			printf("fatal error: fader %d is missing!\n", i);
			panic("missing fader bitmap");
		}
	}
	if (!die) printf(" faders loaded successfully\n");
}

// load all stationary world objects
void WORLD::loadStationaries(const char* filename) {
	// temporary variables
	FILE* file = fopen(filename, "r");
	char line[1024];
	char tmp[MAX_TILE_NAME_LENGTH], tmpFilename[MAX_TILE_NAME_LENGTH + 25], type[MAX_TILE_NAME_LENGTH] = "base";
	STATIONARY tmpObj;
	BITMAP* tmpBitmap;
	int i;
	stationaryMaxWidth = 0;

	// read file
	if (!file) {
		printf("fatal error: World data file does not exist\n");
		panic("missing world data");
	}
	else {
		printf("Loading world objects: \"%s\"\n", filename);
		while (!feof(file)) { // loop though file
			fgets(line, 1024, file);
			if (skipline(line)) continue;
			if (feof(file)) break;
			sscanf(line, "%s", tmp);
			strcpy(type, tmp); // store type
			strcpy(tmpObj.type, type); // set tmpObj type
			i=0;
			stationary_types.push_back(string(type));
			printf(" %s: loading all \"data/world/%s/%%u.bmp\"\n", type, tmp);
			while (1) {
				sprintf(tmpFilename, "data/world/%s/%u.bmp", tmp, i++);
				tmpBitmap = load_bitmap(tmpFilename, NULL);
				if (!tmpBitmap) break;
				tmpObj.set(tmpBitmap);
				stationaryList.push_back(tmpObj);
				stationaryList[stationaryList.size()-1].setPerm();
				if (tmpBitmap->w > stationaryMaxWidth) stationaryMaxWidth = tmpBitmap->w; // set max height/width loaded
				else if (tmpBitmap->h > stationaryMaxWidth) stationaryMaxWidth = tmpBitmap->h;
			}
			printf("  found: %u\n", i-1);
		}
	}
}

// loads tile data from a file - loads the images themselves from "data/tiles/<name>.bmp"
// only call once, it doesn't delete old data
void WORLD::loadTiles(const char* filename) {
	// temporary variables
	FILE* file = fopen(filename, "r");
	char line[1024];
	char tmp[MAX_TILE_NAME_LENGTH], tmpFilename[MAX_TILE_NAME_LENGTH + 20], type[MAX_TILE_NAME_LENGTH] = "base";
	int i = 0;
	bool blocked = false;
	TILE tmpTile;

	// read file
	if (!file) {
		printf("fatal error: Tile data file does not exist\n");
		panic("missing tile data");
	}
	else {
		printf("Loading tile data: \"%s\"\n", filename);
		while (!feof(file)) { // loop though file
			fgets(line, 1024, file);
			if (skipline(line)) continue; // skip comments and blank lines
			sscanf(line, " %s", tmp);
			if (!feof(file) && tmp[0] != '#' && strlen(line)) {
				if (!strcmp(tmp, "tiletype") || !strcmp(tmp, "obstruction")) { // set tile type
					sscanf(line, "%*s \"%s\"", type);
					if (tmp[0] == 'o') blocked = true;
					else blocked = false;
					type[strlen(type)-1] = '\0';
					tile_types.push_back(string(type));
					printf(" type: \"%s\" (%s)\n", type, blocked ? "obstructed" : "passable"); // new type
				}
				else { // load tile
					strcpy(tmpTile.name, tmp); // store name
					strcpy(tmpTile.type, type); // store type
					sprintf(tmpFilename, "data/tiles/%s.bmp", tmp); // form name into a filename
					tmpTile.depth = i; // set depth
					tmpTile.blocked = blocked;
					if (!(tmpTile.texture = load_bitmap(tmpFilename, NULL))) {
						printf(" warning: \"%s\" does not exist!\n", tmpFilename); // bitmap file doesn't exist
					}
					else {
						printf("  found: \"%s\" (#%d)\n", tmpFilename, i+1); // bitmap exists
						tileList.push_back(tmpTile);
						tileList[tileList.size()-1].setPerm();
						i++;
					}
				}
			}
		}
		if (!tileList.size()) { // tileListLength is zero, things are going to either seg fault or abort
			printf("fatal error: No tiles found!\n");
			panic("missing data, no tiles found");
		}
	}
	fclose(file);
}

void WORLD::save_tiles(FILE * fout)
{
    for (int x=0; x<width; x++)
		for (int y=0; y<height; y++)
		    fprintf(fout, "%s\n", tile[x * height + y]->name);

	fputs("end\n", fout);
}

void WORLD::save_stationaries(FILE * fout)
{
    for (int x=0; x<width; x++)
		for (int y=0; y<height; y++)
		    if (stationary[x * height + y]) fprintf(fout, "%s\n%d %d\n", stationary[x * height + y]->type, x, y);

	fputs("end\n", fout);
}

void WORLD::load_tiles(FILE * fin)
{
    char buf[256];

    for (int x=0; x<width; x++) {
		for (int y=0; y<height; y++) {
		    do {
		        fgets(buf, 256, fin);
		    } while (strlen(buf) < 3);
		    strip_nonprintables(buf);
		    setTile(x, y, buf);
		}
    }
}

void WORLD::load_stationaries(FILE * fin)
{
    int tx, ty;
    char buf[256];

    for (int x=0; x<width; x++) {
		for (int y=0; y<height; y++) {
		    do {
		        fgets(buf, 256, fin);
		    } while (strlen(buf) < 3);
		    if (!strncmp(buf, "end", 3) || feof(fin)) return;
		    strip_nonprintables(buf);
		    fscanf(fin, "%d %d\n", &tx, &ty);
		    placeObj(tx, ty, buf);
		}
    }
}

////////////////////////     RENDERING     /////////////////////////////////////////////////////////////////////

inline int col_avg(int c1, int c2) {
	return makecol((getr(c1) + getr(c2)) / 2, (getg(c1) + getg(c2)) / 2, (getb(c1) + getb(c2)) / 2);
}

inline int col_scale(int c, int scale) {
	return makecol((getr(c) * scale) / 100, (getg(c) * scale) / 100, (getb(c) * scale) / 100);
}

// render a minimap onto a bitmap
void WORLD::renderMiniMap(BITMAP* bitmap) {
	clear_to_color(bitmap, makecol(255,0,255));
	miniMapScale = ((double) width / bitmap->w > (double) height / bitmap->h) ? (double) (bitmap->w - 2) / width : (double) (bitmap->h - 2) / height;
	static signed char offsets[9][2] = {{0,0},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1},{0,1},{1,1}};
	int color, tmpC, tmpRand;
	double ppX = (bitmap->w > width) ? (bitmap->w / width + 0.5) : 1; // pixels per x
	double ppY = (bitmap->h > height) ? (bitmap->h / height + 0.5) : 1; // pixels per y
	STATIONARY* tmpSta;
	TILE* tmpTile = tile[0];
	int fudge_factor = (miniMapScale > 1.) ? 0 : 2;
	srand(0);
	for (int x=0; x<width; x++) {
		for (int y=0; y<height; y++) {
			// draw tile to minimap
			for (int sx=0; (double)sx < ppX; sx++) { // for the case that the minimap dimensions are larger than the world dimensions
				for (int sy=0; (double)sy < ppY; sy++) {
				    // initial color
                    if (tmpTile = tile[x * height + y]) tmpC = getpixel(tmpTile->texture, rand()%tmpTile->texture->w, rand()%tmpTile->texture->h);
					// draw object color
					if (tmpSta = stationary[x * height + y]) {
						color = getpixel(tmpSta->img, tmpSta->img->w / 2, tmpSta->img->h / 2);
						if (color != makecol(255,0,255)) {
                            for (int ix=0; ix<TILES_PER_STATIONARY + fudge_factor; ix++) {
                                for (int iy=0; iy<TILES_PER_STATIONARY + fudge_factor; iy++) {
                                    tmpRand = 80 + rand()%40;
                                    putpixel(bitmap, (int)((x + ix) * miniMapScale) + sx - 2, (int)((y + iy) * miniMapScale) + sy - 1, col_scale(col_avg(color, tmpC), tmpRand));
                                }
                            }
						}
					} else if (miniMapScale > 1. && getpixel(bitmap, (int)(x*miniMapScale) + sx + 1, (int)(y*miniMapScale) + sy + 1) != makecol(255,0,255)) {
                        // don't draw over stationary
					} else if (tmpTile = tile[x * height + y]) { // draw tile color
						color = getpixel(tmpTile->texture, rand()%tmpTile->texture->w, rand()%tmpTile->texture->h);
						putpixel(bitmap, (int)(x*miniMapScale) + sx + 1, (int)(y*miniMapScale) + sy + 1, col_avg(color, tmpC));
					}
				}
			}
			// blend colors
			if (x && y + 1 < height) tmpC = col_avg(color, getpixel(tile[(x-1) * height + y + 1]->texture, tileSize/2, tileSize/2));
			else tmpC = color;
		}
	}
	rect(bitmap, 0, 0, (int)(width*miniMapScale + 0.5) + 1, (int)(height*miniMapScale + 0.5) + 1, makecol(0,0,0));
	srand(time(0));
}

// render tiles (faded and everything)
void WORLD::render(BITMAP* bitmap, int destX, int destY, int width, int height, int worldX, int worldY)
{
    // set all variables needed
	int xTile = worldX / tileSize;
	int yTile = worldY / tileSize;
	int worldOffX = worldX % tileSize;
	int worldOffY = worldY % tileSize;
	TILE* currentTile;
	int drawX = (int)((double)(width + worldOffX) / tileSize) + 1;
	int drawY = (int)((double)(height + worldOffY) / tileSize) + 1;
	int offsetX, offsetY, blitX, blitY;
	// do the drawing
	for (int x=0; x<drawX; x++) {
		if (xTile + x < 0 || xTile + x >= this->width) continue;
		offsetX = ((xTile + x) % TILES_PER_TEXTURE) * tileSize;
		for (int y=0; y<drawY; y++) {
			if (yTile + y < 0 || yTile + y >= this->height) continue;
			offsetY = ((yTile + y) % TILES_PER_TEXTURE) * tileSize;
			// tmp drawing variables
			blitX = x * tileSize - worldOffX + destX;
			blitY = y * tileSize - worldOffY + destY;
			// bounds checking
			if (blitX + tileSize < 0 || blitY + tileSize < 0 || blitX - tileSize > bitmap->w || blitY - tileSize > bitmap->h) continue;
			// Tile Drawing
			drawTile(bitmap, xTile + x, yTile + y, blitX, blitY, offsetX, offsetY);
		}
	}
}

// fills a list with all stationary objects inside the passed coordinates
void WORLD::getStationary(list<COORDINATE>* st, int width, int height, int worldX, int worldY) {
	// take account of stationaryMaxWidth
	width += stationaryMaxWidth - tileSize / 2;
	height += stationaryMaxWidth - tileSize / 2;
	worldX -= stationaryMaxWidth / 2 - tileSize / 2;
	worldY -= stationaryMaxWidth / 2 - tileSize / 2;
	// set all variables needed
	int xTile = worldX / tileSize;
	int yTile = worldY / tileSize;
	int worldOffX = worldX % tileSize;
	int worldOffY = worldY % tileSize;
	STATIONARY* current;
	int drawX = (width + worldOffX) / tileSize + 1;
	int drawY = (height + worldOffY) / tileSize + 1;
	int blitX, blitY;
	st->clear(); // clear the list
	// generate list
	for (int x=0; x<drawX; x++) {
		if (xTile + x < 0 || xTile + x >= this->width) continue;
		for (int y=0; y<drawY; y++) {
			if (yTile + y < 0 || yTile + y >= this->height) continue;
			current = stationary[(x + xTile) * this->height + y + yTile]; // make line more efficient
			if (!current) continue;
			st->push_front(COORDINATE(x + xTile, y + yTile, current->depth));
		}
	}
	st->sort(); // sort so that the smallest trees are on the bottom
}

// renders the stationary objects in a list
void WORLD::renderStationary(BITMAP* bitmap, list<COORDINATE>* st, int destX, int destY, int width, int height, int worldX, int worldY) {
	// set all variables needed
	int xTile = worldX / tileSize;
	int yTile = worldY / tileSize;
	int worldOffX = worldX % tileSize;
	int worldOffY = worldY % tileSize;
	STATIONARY* current;
	int blitX, blitY;
	list<COORDINATE>::iterator p;
	// loop through
	for (p=st->begin(); p!=st->end(); p++) {
		current = stationary[p->x * this->height + p->y]; // make line more efficient
		blitX = (p->x - xTile) * tileSize - worldOffX + destX + tileSize / 2;
		blitY = (p->y - yTile) * tileSize - worldOffY + destY + tileSize / 2;
		// Drawing
		if (blitX + current->img->w / 2 > destX && blitY + current->img->h / 2 > destY && blitX < destX + width + current->img->h / 2 && blitY < destY + height + current->img->h / 2) {
			draw_sprite(bitmap, current->img, blitX - current->img->w / 2, blitY - current->img->h / 2);
		}
	}
}

// blits a bitmap onto a destination using a third image for the alphas of the bitmap
void faded_blit(BITMAP* bitmap, BITMAP* dest, BITMAP* alpha, int offsetX, int offsetY, int x, int y) {
	int bitX, bitY, col_dest, col_bit, tmp, tmp_in;
	for (bitX = 0; bitX < alpha->w; bitX++) {
		for (bitY = 0; bitY < alpha->h; bitY++) {
			if ((tmp = getpixel(alpha, bitX, bitY) & 0xff) && tmp != 0xff) { // needs alpha blending done between pixels (don't change this line! it is most efficient how it is right now!)
				col_dest = getpixel(dest, x + bitX, y + bitY);
				if (col_dest != -1) { // only if the destination pixel exists
					tmp_in = 0xff - tmp;
					col_bit = getpixel(bitmap, bitX + offsetX, bitY + offsetY);
					putpixel(dest, x + bitX, y + bitY, makecol((getr(col_dest) * tmp_in + getr(col_bit) * tmp) / 255, (getg(col_dest) * tmp_in + getg(col_bit) * tmp) / 255, (getb(col_dest) * tmp_in + getb(col_bit) * tmp) / 255));
				}
			}
			else if (tmp) { // just put the bitmap's pixel, no fading
				putpixel(dest, x + bitX, y + bitY, getpixel(bitmap, bitX + offsetX, bitY + offsetY));
			}
			// else {the dest pixel should not change}
		}
	}
}

// same as faded_blit() except very optimized for drawing to/from 32 bit bitmaps. Will not work for any other color depths (without modification).
void faded_blit_32(BITMAP* bitmap, BITMAP* dest, BITMAP* alpha, int offsetX, int offsetY, int x, int y) {
	int bitX, bitY, col_dest, col_bit, tmp, tmp_in;
	unsigned char* ptrl_get, *ptrl_put;
	for (bitX = 0; bitX < alpha->w; bitX++) {
		for (bitY = 0; bitY < alpha->h; bitY++) {
			ptrl_get = bitmap->line[bitY + offsetY];
			ptrl_put = dest->line[y + bitY];
			if ((tmp = _getpixel32(alpha, bitX, bitY) & 0xff) && tmp != 0xff) {
				col_dest = getpixel(dest, x + bitX, y + bitY);
				if (col_dest != -1) { // only if the destination pixel exists
					tmp_in = 0xff - tmp;
					col_bit = ((long*)ptrl_get)[bitX + offsetX];
					((long*)ptrl_put)[x + bitX] = ( (( ((col_dest >> 16) & 0xff) * tmp_in + ((col_bit >> 16) & 0xff) * tmp) << 8) & 0x00ff0000) | (( ((col_dest >> 8) & 0xff) * tmp_in + ((col_bit >> 8) & 0xff) * tmp) & 0x0000ff00) | ((( ((col_dest) & 0xff) * tmp_in + ((col_bit) & 0xff) * tmp) >> 8) & 0x000000ff);
				}
			}
			else if (tmp) { // just put the bitmap's pixel, no fading
				putpixel(dest, x + bitX, y + bitY, ((long*)ptrl_get)[bitX + offsetX]);
			}
			// else {the dest pixel should not change}
		}
	}
}


// coordinates and initiates all the drawing for a tile (which fades to draw and in which order, then calls faded_blit with the correct args)
void WORLD::drawTile(BITMAP* buffer, int x, int y, int destX, int destY, int offsetX, int offsetY) {
	// offsets and init
	static signed char offsets[9][2] = {{0,0},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1},{0,1},{1,1}};
	bool is_same = true;
	int depthOrdered[8][2]; // {id,depth}
	char k, m;
	static bool hj=false;
	int tmpOffX, tmpOffY;
	char drawmask[8] = {0,0,0,0,0,0,0,0};

	// load tile surroundings
	TILE* tile[9];
	TILE* tmpTile;
	for (char i=0; i<9; i++) {
		tmpTile = getTile(x + offsets[i][0], y + offsets[i][1]);
		tile[i] = tmpTile ? tmpTile : tile[0];
		if (tile[i]->texture != tile[0]->texture) is_same = false;
		if (i) { // generate depthOrdered, aka. put them in order by depth
			k = 0;
			while (depthOrdered[k][1] >= tile[i]->depth && k < i - 1) k++; // find insert point
			m = i < 8 ? i : 7; // shift over
			while (m > k) {
				depthOrdered[m][0] = depthOrdered[m-1][0];
				depthOrdered[m][1] = depthOrdered[m-1][1];
				m--;
			}
			depthOrdered[k][0] = i; // add to depthOrdered
			depthOrdered[k][1] = tile[i]->depth;
		}
	}

	// drawing - needs to be efficient
	blit(tile[0]->texture, buffer, offsetX, offsetY, destX, destY, tileSize, tileSize);
	if (is_same) {
		return;
	}
	else if (global_config.CONFIG_FADE_TILES) { // draw faded sections
		char i;
		// determine what to draw (so that we don't double-draw faders), looks better without doing this, but this is faster
		for (char j=1; j<9; j++) {
			i = depthOrdered[8-j][0];
			if (tile[i]->depth > tile[0]->depth && tile[i]->texture != tile[0]->texture) {
				if (tile[i]->depth > tile[0]->depth && tile[i]->texture != tile[0]->texture) {
					if (abs(offsets[i][0] + offsets[i][1]) == 1 && tile[(i-2 > 0) ? (i-2) : 7]->texture == tile[i]->texture) { // 45 degree edge
						drawmask[i - 1] = 3;
#if LOW_QUALITY_TILE_FADING
							if (drawmask[(i-3 >= 0) ? (i-3) : 6] < 2) drawmask[(i-3 >= 0) ? (i-3) : 6] = -1;
#endif
					}
					else if (abs(offsets[i][0] + offsets[i][1]) == 1 && (tile[i+1]->texture == tile[i]->texture || tile[(i-1) ? (i-1) : 8]->texture == tile[i]->texture)) { // straight edge
						drawmask[i - 1] = 2;
#if LOW_QUALITY_TILE_FADING
							if (tile[i+1]->texture == tile[i]->texture) drawmask[i] = -1;
							if (tile[(i-1) ? (i-1) : 8]->texture == tile[i]->texture) drawmask[(i-2 >= 0) ? (i-2) : 7] = -1;
#endif
					}
					else if (drawmask[i - 1] != - 1) { // other edge
						drawmask[i - 1] = 1;
					}
				}
			}
		}
		// draw it all
		for (char j=1; j<9; j++) {
			i = depthOrdered[8-j][0];
			if (drawmask[i - 1] > 0) {
				if (drawmask[i - 1] == 3) { // 45 degree edge
					faded_blit_32(tile[i]->texture, buffer, fader[i / 2 + 12], offsetX, offsetY, destX, destY);
				}
				else if (drawmask[i - 1] == 2) { // straight edge
					if (offsets[i][0] > 0) tmpOffX = tileSize - fader[i / 2 + 8]->w;
					else tmpOffX = 0;
					if (offsets[i][1] > 0) tmpOffY = tileSize - fader[i / 2 + 8]->h;
					else tmpOffY = 0;
					faded_blit_32(tile[i]->texture, buffer, fader[i / 2 + 8], offsetX + tmpOffX, offsetY + tmpOffY, destX + tmpOffX, destY + tmpOffY);
				}
				else { // other edge
					if (offsets[i][0] > 0) tmpOffX = tileSize - fader[i-1]->w;
					else tmpOffX = 0;
					if (offsets[i][1] > 0) tmpOffY = tileSize - fader[i-1]->h;
					else tmpOffY = 0;
					faded_blit_32(tile[i]->texture, buffer, fader[i-1], offsetX + tmpOffX, offsetY + tmpOffY, destX + tmpOffX, destY + tmpOffY);
				}
			}
		}
	}
}

