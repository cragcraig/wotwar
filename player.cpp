#include "wotwar.h"

PLAYER::PLAYER(void) : popup(NULL), faction(0), group_id_count(1), active(false), unit_count(0), can_die(false), resources(0)
{
	clear_select();
}

PLAYER::~PLAYER(void)
{
    if (popup) delete popup;
}

void PLAYER::init(int id, int faction, int start_resources)
{
	this->id = id;
	this->faction = faction;
	resources = start_resources;
	color = new_color(id);
}

bool PLAYER::is_active(void)
{
    return active;
}

bool PLAYER::is_host(void)
{
    return (id == 1);
}

void PLAYER::set_active(void)
{
    active = true;
}

int PLAYER::get_color(void)
{
	return color;
}

int PLAYER::get_faction(void)
{
	return faction;
}

void PLAYER::set_faction(int to_civ)
{
    faction = to_civ;
}

int PLAYER::new_group_id(void)
{
    return group_id_count++;
}

void PLAYER::unit_decrement(void)
{
    unit_count--;
}

void PLAYER::unit_increment(void)
{
    unit_count++;
    can_die = true;
}

int PLAYER::units_alive(void)
{
    return unit_count;
}

bool PLAYER::is_dead(void)
{
    return (unit_count <= 0 && can_die);
}

void PLAYER::select_area(int x1, int y1, int x2, int y2)
{
	sel.select = true;
	sel.selectX = x1;
	sel.selectY = y1;
	sel.selectBoxX = x2;
	sel.selectBoxY = y2;
}

void PLAYER::check_select(GROUP* pAgent)
{
	if (sel.select) {
		// Click select or box select (needs to be attached to a specific player)
		if (sel.selectX == sel.selectBoxX && sel.selectY == sel.selectBoxY) { // single-click select
			sel.tmpDist = pAgent->dist_to_agent(sel.selectX, sel.selectY);
			if (sel.tmpDist < AGENT_SELECT_DIST && sel.tmpDist < sel.selectDist) {
				sel.agent_to_select = pAgent;
				sel.selectDist = sel.tmpDist;
			}
		}
		if (!pAgent->is_building() && pAgent->in_box(sel.selectX, sel.selectY, sel.selectBoxX, sel.selectBoxY) && (sel.selectX != sel.selectBoxX || sel.selectY != sel.selectBoxY)) pAgent->select(true); // box select
		else if (!KEYS_ADD_TO_SELECTED && sel.agent_to_select != pAgent) pAgent->select(false); // unselect
	}
}

void PLAYER::update_select(void)
{
	if (sel.agent_to_select) sel.agent_to_select->select(true);
	clear_select();
}

void PLAYER::clear_select(void)
{
	sel.select = false;
	sel.agent_to_select = NULL;
	sel.selectDist = 99999;
}

int PLAYER::new_color(int id)
{
	static int colors[] = {makecol(255,255,255), makecol(0,0,200), makecol(0,200,0), makecol(200,0,0), makecol(0,200,200), makecol(200,200,0), makecol(200,0,200), makecol(0,100,200), makecol(200,0,100)};
	return colors[id];
}

void PLAYER::draw_selected(DISPLAY* disp)
{
	list<GROUP*>::iterator i;
	for (i = selected.begin(); i != selected.end(); i++) {
		(*i)->draw_selected(disp);
	}
}

void PLAYER::draw_menu(DISPLAY* disp, AGENT_HANDLER* ah)
{
    if (selected.size() && (*selected.begin())->is_building()) {
        if (!popup) popup = new POPUP((*selected.begin())->get_buildable(), ah, id);
        popup->draw(disp, disp->toScreenX((*selected.begin())->get_x()), disp->toScreenY((*selected.begin())->get_y()));
    }
    //textprintf_ex(disp->buffer, font, 5, 20, makecol(255,255,255), makecol(0,0,0), "units: %d", units_alive()); // debug, draw number of units
}

void PLAYER::add_selected(GROUP * ap)
{
	selected.push_back(ap);
}

void PLAYER::remove_selected(GROUP * ap)
{
	selected.remove(ap);
	if (popup) {
	    delete popup;
	    popup = NULL;
	}
}

void PLAYER::add_resources(int n)
{
    resources += n;
    if (resources < 0) resources = 0;
}

int PLAYER::get_resources(void)
{
    return resources;
}

void PLAYER::subtract_resources(int n)
{
    add_resources(-1*n);
}

GROUP* PLAYER::get_selected_building(void)
{
    if (selected.size() && selected.front()->is_building()) return selected.front();
    return NULL;
}
