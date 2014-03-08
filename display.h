#ifndef DISPLAY_H
#define DISPLAY_H

#include "wotwar_classes.h"
#include <list>
#include <string>

class DISPLAY_MSG
{
    public:
        string msg;
        int counter;
        DISPLAY_MSG(string str) : msg(str), counter(MAX_ONSCREEN_MSGTIME*GOAL_FPS) {}
};

class DISPLAY {

	private:
		int worldX, worldY, displayHeight, displayWidth;
		int displayX, displayY;
		int active_player;
		PLAYER* player_link;
		double worldToMiniMapScale;
		int miniMapX, miniMapY, miniMapWidth, miniMapHeight;
		WORLD* world;
		BITMAP* videomemory_buffer;
		bool videomem_valid;
		BITMAP* worldRender;
		BITMAP* overRender;
		BITMAP* arrow;
		BITMAP* miniMap;
		int offsetX, offsetY;
		void renderWorld(void);
		void shiftRenderWorld(void);
		void setupMiniMap(void);
		list<DISPLAY_MSG> messages;

		// enter message
		void enterMessage();
		int enter_state;
		int enter_pos;
		char enter_msg[PLAYER_MESSAGE_MAXLENGTH];

	public:
		BITMAP*& buffer;
		list<COORDINATE> stationaries;
		void drawWorld(void);
		void drawSelectBox(int x1, int y1, int x2, int y2);
		void drawOverlay(GAME* game, AGENT_HANDLER* pAh);
		void drawOverhead();
		void getOverhead();
		void drawMessages();
		void worldScroll(int horizontalDistance, int verticalDistance);
		void worldGoto(int x, int y);
		void worldCenterOver(int x, int y);
		void drawArrow(int x, int y, int theta);
		bool isOverOverlay(int x, int y);
		bool isOverMiniMap(int x, int y);
		void overlayClick(int x, int y);
		int get_active_player(void);
		int miniMapToWorldX(int x);
		int miniMapToWorldY(int y);
		void updateMiniMap(void);
		void redrawTiles(int tx, int ty, int w, int h);
		void addMessage(const char * str);

		// Get, Set, and Check functions

		inline int getX(void) {
			return displayX;
		}

		inline int getY(void) {
			return displayY;
		}

		inline int getWidth(void) {
			return displayWidth;
		}

		inline int getHeight(void) {
			return displayHeight;
		}

		inline int getWorldX(void) {
			return worldX;
		}

		inline int getWorldY(void) {
			return worldY;
		}

		inline int toWorldX(int displayXCoordinate) {
			return worldX + displayXCoordinate - displayX;
		}

		inline int toWorldY(int displayYCoordinate) {
			return worldY + displayYCoordinate - displayY;
		}

		inline int toScreenX(int worldXCoordinate) {
			return worldXCoordinate - worldX + displayX;
		}

		inline int toScreenY(int worldYCoordinate) {
			return worldYCoordinate - worldY + displayY;
		}

		inline void setWorldPos(int x, int y) {
			worldX = x > 0 ? x : 0;
			worldY = y > 0 ? x : 0;
		}

		inline bool isOnScreen(int x, int y) {
			if (x > worldX && x < worldX + displayWidth && y > worldY && y < worldY + displayHeight) return true;
			else return false;
		}

		DISPLAY(BITMAP*& buffer, BITMAP* videomemory_buffer, WORLD* world, PLAYER* player_link, int player, int x, int y, int width, int height);
		virtual ~DISPLAY(void);
};

#endif
