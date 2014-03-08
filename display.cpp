#include "wotwar.h"
#include <cctype>

// draw worldRender at the correct offset
void DISPLAY::drawWorld(void) {

	// if using video memory to hold a buffer
	if (videomemory_buffer && !videomem_valid) {
	    blit(worldRender, videomemory_buffer, 0, 0, 0, 0, worldRender->w, worldRender->h);
	    videomem_valid = true;
	}

    // draw to screen
	blit(videomemory_buffer ? videomemory_buffer : worldRender, buffer, WORLD_RENDER_BORDER_SIZE + offsetX, WORLD_RENDER_BORDER_SIZE + offsetY, displayX, displayY, displayWidth, displayHeight);
}

// draw trees, etc.
void DISPLAY::drawOverhead(void) {
	if (stationaries.size() > NUM_OF_STATIONARIES_TO_SWITCH_DRAWING_MODE) masked_blit(overRender, buffer, WORLD_RENDER_BORDER_SIZE + offsetX, WORLD_RENDER_BORDER_SIZE + offsetY, displayX, displayY, displayWidth, displayHeight);
	else world->renderStationary(buffer, &stationaries, displayX, displayY, displayWidth, displayHeight, worldX, worldY);
}

void DISPLAY::getOverhead() {
	world->getStationary(&stationaries, displayWidth + 2 * WORLD_RENDER_BORDER_SIZE, displayHeight + 2 * WORLD_RENDER_BORDER_SIZE, worldX - WORLD_RENDER_BORDER_SIZE, worldY - WORLD_RENDER_BORDER_SIZE);
	if (stationaries.size() > NUM_OF_STATIONARIES_TO_SWITCH_DRAWING_MODE) {
		clear_to_color(overRender, makecol(255,0,255));
		world->renderStationary(overRender, &stationaries, offsetX, offsetY, overRender->w, overRender->h, worldX - WORLD_RENDER_BORDER_SIZE, worldY - WORLD_RENDER_BORDER_SIZE);
	}
}

// messages
void DISPLAY::drawMessages()
{
    // draw messages
    int i = 0;
    list<DISPLAY_MSG>::iterator p;
	for (p = messages.begin(); p != messages.end(); p++) {
	    textout_ex(buffer, font, p->msg.c_str(), 15, 15 + 1.5 * text_height(font) * i++, makecol(200,200,200), -1);
	    p->counter--;
	}

	// delete old messages
	if (!messages.empty() && messages.front().counter <= 0) messages.pop_front();
}

void DISPLAY::addMessage(const char * str)
{
    messages.push_back(DISPLAY_MSG(str));
    if (messages.size() > MAX_ONSCREEN_MESSAGES) messages.pop_front();
}

// full rerender of the world
void DISPLAY::renderWorld(void) {
	world->render(worldRender, 0, 0, worldRender->w, worldRender->h, worldX - WORLD_RENDER_BORDER_SIZE, worldY - WORLD_RENDER_BORDER_SIZE);
	offsetX = 0;
	offsetY = 0;
	videomem_valid = false;
}

// more efficient rerender of the world so long as you have shifted less than (2 * WORLD_RENDER_BORDER_SIZE) since the last render (aka. you are scrolling)
void DISPLAY::shiftRenderWorld(void) {
	int xPos, yPos, yWidth, extraX, extraY, tmpOffsetX, tmpOffsetY, shiftX, shiftY;
	// determine how far to shift the old worldRender
	if (abs(offsetX) >= WORLD_RENDER_BORDER_SIZE) { // shiftX
		tmpOffsetX = offsetX % WORLD_RENDER_BORDER_SIZE;
		if (offsetX > 0) shiftX = WORLD_RENDER_BORDER_SIZE;
		else shiftX = -WORLD_RENDER_BORDER_SIZE;
	}
	else {
		shiftX = 0;
		tmpOffsetX = offsetX;
	}
	if (abs(offsetY) >= WORLD_RENDER_BORDER_SIZE) { // shiftY
		tmpOffsetY = offsetY % WORLD_RENDER_BORDER_SIZE;
		if (offsetY > 0) shiftY = WORLD_RENDER_BORDER_SIZE;
		else shiftY = -WORLD_RENDER_BORDER_SIZE;
	}
	else {
		shiftY = 0;
		tmpOffsetY = offsetY;
	}
	// shift the old render
	blit(worldRender, worldRender, shiftX, shiftY, 0, 0, worldRender->w, worldRender->h);
	// do new renders
	if (abs(offsetX) >= WORLD_RENDER_BORDER_SIZE) { // Render X
		if (offsetX > 0) world->render(worldRender, worldRender->w - WORLD_RENDER_BORDER_SIZE + tmpOffsetX, offsetY, 2*WORLD_RENDER_BORDER_SIZE + tmpOffsetX, worldRender->h, worldX + worldRender->w - 2 * WORLD_RENDER_BORDER_SIZE, worldY - WORLD_RENDER_BORDER_SIZE);
		else world->render(worldRender, tmpOffsetX, offsetY, WORLD_RENDER_BORDER_SIZE - tmpOffsetX, worldRender->h, worldX - WORLD_RENDER_BORDER_SIZE, worldY - WORLD_RENDER_BORDER_SIZE);
	}
	if (abs(offsetY) >= WORLD_RENDER_BORDER_SIZE) { // RenderY
		if (offsetY > 0) world->render(worldRender, offsetX, worldRender->h - WORLD_RENDER_BORDER_SIZE + tmpOffsetY, worldRender->w, WORLD_RENDER_BORDER_SIZE + tmpOffsetY, worldX - WORLD_RENDER_BORDER_SIZE, worldY + worldRender->h - 2 * WORLD_RENDER_BORDER_SIZE);
		else world->render(worldRender, offsetX, tmpOffsetY, worldRender->w, WORLD_RENDER_BORDER_SIZE - tmpOffsetY, worldX - WORLD_RENDER_BORDER_SIZE, worldY - WORLD_RENDER_BORDER_SIZE);
	}
	offsetX = tmpOffsetX;
	offsetY = tmpOffsetY;
	videomem_valid = false;
}

void DISPLAY::drawSelectBox(int x1, int y1, int x2, int y2) {
	rect(buffer, x1 + 1, y1 + 1, x2 + 1, y2 + 1, makecol(255, 255, 255));
	rect(buffer, x1, y1, x2, y2, makecol(0, 0, 0));
}

void DISPLAY::drawOverlay(GAME* game, AGENT_HANDLER* pAh) {
	// Remove this next bit once menus have been added, there is no point to drawing black underneath them.
	// For now it is just to make for a pretty clipped viewport without funny relics. Could have also done this more efficiently using a sub bitmap.
	/*
	int fillColor = makecol(0, 0, 0);
	rectfill(buffer, 0, 0, buffer->w, displayY - 1, fillColor);
	rectfill(buffer, 0, displayY, displayX, displayY + displayHeight, fillColor);
	rectfill(buffer, displayX + displayWidth, displayY, buffer->w, displayY + displayHeight, fillColor);
	rectfill(buffer, 0,  displayY + displayHeight, buffer->w, buffer->h, fillColor);
    */

	// Mini Map
	draw_sprite(buffer, miniMap, miniMapX, miniMapY);
	pAh->drawAllToMiniMap(miniMapX, miniMapY, worldToMiniMapScale);

	// draw screenbox on minimap
	rect(buffer, (int) (worldToMiniMapScale * worldX) + miniMapX + 1, (int) (worldToMiniMapScale * worldY) + miniMapY + 1, (int) (worldToMiniMapScale * worldX) + (int)(worldToMiniMapScale * displayWidth) + miniMapX + 1, (int) (worldToMiniMapScale * worldY) + (int)(worldToMiniMapScale * displayHeight) + miniMapY + 1, makecol(170,170,170));

    // add messages
    if (key[KEY_ENTER] && !enter_state) {
        // begin message
        enter_state = 1;
        clear_keybuf();
        enter_pos = 0;
        enter_msg[0] = '\0';
    } else if (!key[KEY_ENTER] && (enter_state == 1 || enter_state == 2)) {
        // add to message
        enter_state = 2;
        enterMessage();
    } else if (key[KEY_ENTER] && enter_state == 2) {
        // commit message
        enter_state = 3;
        if (enter_pos) {
            addMessage(enter_msg);
            game->sendMessage(enter_msg);
        }
    } else if (!key[KEY_ENTER] && enter_state == 3) {
        enter_state = 0;
    }
}

void DISPLAY::enterMessage()
{
    // draw
    textout_centre_ex(buffer, font, enter_msg, buffer->w/2 + 1, buffer->h/2 - text_height(font)/2 + 1, makecol(0,0,0), -1);
    textout_centre_ex(buffer, font, enter_msg, buffer->w/2, buffer->h/2 - text_height(font)/2, makecol(255,255,255), -1);
    int len = text_length(font, enter_msg);
    rect(buffer, buffer->w/2 - len/2 - 4, buffer->h/2 - text_height(font)/2 - 2, buffer->w/2 + len/2 + 6, buffer->h/2 + text_height(font)/2 + 4, makecol(0,0,0));
    rect(buffer, buffer->w/2 - len/2 - 5, buffer->h/2 - text_height(font)/2 - 3, buffer->w/2 + len/2 + 5, buffer->h/2 + text_height(font)/2 + 3, makecol(255,255,255));

    char c;
    if (keypressed()) {
        c = readkey();
        if (isprint(c) && enter_pos < PLAYER_MESSAGE_MAXLENGTH - 1) {
            enter_msg[enter_pos++] = c;
        } else if (c == '\b' && enter_pos > 0) {
            enter_pos--;
        }
        enter_msg[enter_pos] = '\0';
    }
}

void DISPLAY::redrawTiles(int tx, int ty, int w, int h)
{
    if (tx < 0) tx = 0;
    if (ty < 0) ty = 0;
    if (tx + w >= world->getMapWidth()) tx = world->getMapWidth() - w - 1;
    if (ty + h >= world->getMapHeight()) ty = world->getMapHeight() - h - 1;

    int dx = toScreenX(tx * TILE_SIZE) - displayX;
    int dy = toScreenY(ty * TILE_SIZE) - displayY;
    int dw = (w + 2) * TILE_SIZE;
    int dh = (h + 2) * TILE_SIZE;
    int wx = tx * TILE_SIZE - offsetX - TILE_SIZE;
    int wy = ty * TILE_SIZE - offsetY - TILE_SIZE;

    world->render(worldRender, dx, dy, dw, dh, wx, wy);
    videomem_valid = false;
}

void DISPLAY::worldScroll(int horizontalDistance, int verticalDistance) {
	// X movement
	if (horizontalDistance < 0 && worldX < abs(horizontalDistance)) horizontalDistance = -worldX;
	else if (horizontalDistance > 0 && world->getWidth() - (worldX + displayWidth) - 1 < horizontalDistance) horizontalDistance = world->getWidth() - (worldX + displayWidth) - 1;
	worldX += horizontalDistance;
	if (worldX > world->getWidth()) worldX = world->getWidth();
	// Y movement
	if (verticalDistance < 0 && worldY < abs(verticalDistance)) verticalDistance = -worldY;
	else if (verticalDistance > 0 && world->getHeight() - (worldY + displayHeight) - 1 < verticalDistance) verticalDistance = world->getHeight() - (worldY + displayHeight) - 1;
	worldY += verticalDistance;
	if (worldY > world->getHeight()) worldY = world->getHeight();
	// Take care of offset changes and rerenderings
	offsetX += horizontalDistance;
	offsetY += verticalDistance;
	if (abs(offsetX) >= WORLD_RENDER_BORDER_SIZE || abs(offsetY) >= WORLD_RENDER_BORDER_SIZE) {
		shiftRenderWorld(); // render new edges
		getOverhead();
	}
}

void DISPLAY::worldGoto(int x, int y) {
	if (x < 0) x = 0;
	else if (x > world->getWidth() - displayWidth) x = world->getWidth() - displayWidth;
	if (y < 0) y = 0;
	else if (y > world->getHeight() - displayHeight) y = world->getHeight() - displayHeight;
	offsetX = 0;//x % WORLD_RENDER_BORDER_SIZE;
	offsetY = 0;//y % WORLD_RENDER_BORDER_SIZE;
	//offsetX *= -1; // added
	//offsetY *= -1; // added
	worldX = x - offsetX; // - offsetX; // removed
	worldY = y - offsetY; // - offsetY; // removed
	renderWorld(); // render new background
	getOverhead(); // trees, etc.
}

void DISPLAY::worldCenterOver(int x, int y)
{
    worldGoto(x - buffer->w/2, y - buffer->h/2);
}

void DISPLAY::setupMiniMap(void) {
	world->renderMiniMap(miniMap);
	worldToMiniMapScale = world->getMiniMapScale() / world->getTileSize();
	miniMapX = displayX + 10;
	miniMapY = displayY + displayHeight - (int)(world->getHeight()*worldToMiniMapScale) - 10;
	miniMapWidth = (int) (world->getWidth() * worldToMiniMapScale);
	miniMapHeight = (int) (world->getHeight() * worldToMiniMapScale);
}

void DISPLAY::updateMiniMap(void)
{
    world->renderMiniMap(miniMap);
}

bool DISPLAY::isOverMiniMap(int x, int y) {
	if (x > miniMapX && y > miniMapY && x < miniMapX + miniMapWidth && y < miniMapY + miniMapHeight) return true;
	return false;
}

bool DISPLAY::isOverOverlay(int x, int y) {
	if (isOverMiniMap(x,y) || (player_link->popup && player_link->popup->which_over(x, y) >= 0)) return true;
	return false;
}

int DISPLAY::miniMapToWorldX(int x) {
	return (int)((x - miniMapX) / worldToMiniMapScale);
}

int DISPLAY::miniMapToWorldY(int y) {
	return (int)((y - miniMapY) / worldToMiniMapScale);
}

void DISPLAY::overlayClick(int x, int y) {
	if (isOverMiniMap(x,y)) worldGoto(miniMapToWorldX(x) - displayWidth/2, miniMapToWorldY(y) - displayHeight/2);
	else if (player_link->popup) player_link->popup->click(x, y);
}

void DISPLAY::drawArrow(int x, int y, int theta) {
	pivot_sprite(buffer, arrow, toScreenX(x), toScreenY(y), -ARROW_OFFSET, arrow->h/2, itofix(theta));
}

int DISPLAY::get_active_player(void)
{
	return active_player;
}

DISPLAY::DISPLAY(BITMAP*& buffer, BITMAP* videomemory_buffer, WORLD* world, PLAYER* player_link, int player, int x, int y, int width, int height) : buffer(buffer), videomemory_buffer(videomemory_buffer), player_link(player_link), enter_state(0), enter_pos(0), videomem_valid(false)
{
	this->world = world;
	active_player = player;
	displayX = x;
	displayY = y;
	offsetX = 0;
	offsetY = 0;
	displayWidth = width;
	displayHeight = height;
	arrow = load_bitmap("data/gui/arrow.bmp", NULL);
	if (!arrow) panic("missing 'data/gui/arrow.bmp'!");
	worldRender = create_bitmap(displayWidth + 2 * WORLD_RENDER_BORDER_SIZE, displayHeight + 2 * WORLD_RENDER_BORDER_SIZE);
	overRender = create_bitmap(displayWidth + 2 * WORLD_RENDER_BORDER_SIZE, displayHeight + 2 * WORLD_RENDER_BORDER_SIZE);
	miniMap = create_bitmap(150, 150);
	worldX = 0;
	worldY = 0;
	getOverhead();
	renderWorld();
	setupMiniMap();
}

DISPLAY::~DISPLAY(void) {
	destroy_bitmap(worldRender);
	destroy_bitmap(miniMap);
	destroy_bitmap(arrow);
}
