#include "wotwar.h"
#include <limits>

void AGENT_HANDLER::updateAll(bool draw)
{
	GROUP* pAgent = pFirst;
	PLAYER* pl = NULL;

	// draw selected units
	if (draw) player[display->get_active_player()].draw_selected(display);

	// loop through all agents
	while (pAgent != NULL) {
		// Set current player
		pl = &player[pAgent->get_team()];
		// Update unit
		pAgent->update();
		// Possibly select unit
		pl->check_select(pAgent);
		// add tasks / handle messages
		give_agent_tasks(pAgent);
		// Draw
		if (pAgent->is_sel() && KEYS_SHOW_PATHS && draw) pAgent->draw_task_path(display); // Draw path
		if (draw) pAgent->draw(display); // Draw AGENT
		// Next agent
		pAgent = pAgent->pNext;
	}

	// clear any active player selects and do single click select
	for (int i=0; i<player.size(); i++) player[i].update_select();

	// clear task for selected, shift on next turn's messages
	shiftOffTasks();

	// kill off units that have died
	killList();

	// debug only
	// debug_unit_creator();
}

void AGENT_HANDLER::debug_unit_creator(void) // debug only
{
#define CREATE_UNIT_AT_MOUSE(n) {create((n), (key[KEY_1] ? 2 : 1), display->toWorldX(mouse_x), display->toWorldY(mouse_y), 7); pressed = true;}
#define CREATE_BUILD_AT_MOUSE(n) {if (!create_building((n), (key[KEY_1] ? 2 : 1), display->toWorldX(mouse_x), display->toWorldY(mouse_y))) puts("warning: unable to create building"); pressed = true;}
	static bool pressed = false;
	if (!pather->passable(display->toWorldX(mouse_x),display->toWorldY(mouse_y))) /* do nothing, else */;
	else if (key[KEY_B] && !pressed) CREATE_UNIT_AT_MOUSE("Aiel Sis'awa'man")
	else if (key[KEY_N] && !pressed) CREATE_UNIT_AT_MOUSE("Trolloc Crossbowman")
	else if (key[KEY_M] && !pressed) CREATE_UNIT_AT_MOUSE("Myrddraal")
	else if (key[KEY_V] && !pressed) CREATE_UNIT_AT_MOUSE("Warder")
	if (key[KEY_O] && !pressed) {kill(pFirst); pressed = true;}
	if (key[KEY_P] && !pressed) {kill(pLast); pressed = true;}
	if (key[KEY_SPACE] && !pressed) {is_paused = !is_paused; pressed = true;}
	if (pressed && !key[KEY_O] && !key[KEY_P] && !key[KEY_N] && !key[KEY_M] && !key[KEY_B] && !key[KEY_V] && !key[KEY_Z] && !key[KEY_SPACE]) pressed = false;

	if (key[KEY_H] && pFirst) pFirst->get_first_agent()->hurt(1);
}

GROUP* AGENT_HANDLER::get_agent_at(int x, int y)
{
	int tmp;
	int max_dist = numeric_limits<int>::max();
	GROUP* rAgent = NULL;
	GROUP* pAgent = pFirst;

	while (pAgent != NULL) {
		tmp = (int)pAgent->dist_to_agent(x, y);
		if (tmp < AGENT_SELECT_DIST && tmp < max_dist) {
			rAgent = pAgent;
			max_dist = tmp;
		}
		pAgent = pAgent->pNext;
	}
	return rAgent;
}

GROUP* AGENT_HANDLER::get_agent_within(int x, int y, int dist, GROUP* agent_ignore)
{
	GROUP* pAgent = pFirst;

	while (pAgent != NULL) {
		if ((int)pAgent->dist_to(x, y) < pAgent->get_radius() + dist) {
			if (pAgent != agent_ignore) return pAgent;
		}
		pAgent = pAgent->pNext;
	}
	return NULL;
}

// draws all units onto the minimap
void AGENT_HANDLER::drawAllToMiniMap(int x, int y, double scale)
{
	GROUP* pAgent = pLast;
	while (pAgent != NULL) {
		if (pAgent->is_sel() && (key[KEY_LCONTROL] || key[KEY_RCONTROL])) pAgent->draw_path_minimap(display, x, y, scale);
		pAgent->drawToMiniMap(display, x, y, scale);
		pAgent = pAgent->pPrevious;
	}
}

PLAYER* AGENT_HANDLER::get_player(int id)
{
	return &player[id];
}

void AGENT_HANDLER::network_sndtask(TASK* task, GROUP* agent)
{
    // store group id in msg
    if (task->net_msg.type == MSGTYPE_TASK) task->net_msg.msg.task.group_id = agent->get_group_id();
    else if (task->net_msg.type == MSGTYPE_FIGHT) task->net_msg.msg.fight.group_id = agent->get_group_id();
    else return;

    network->broadcast((char*)&task->net_msg, sizeof(task->net_msg));
}

void AGENT_HANDLER::give_agent_tasks(GROUP* pAgent)
{
	TASK* tmpTask, *tmpTaskSub;
	int player_id = pAgent->get_team();

	// Set tasks to AGENTs
	tmpTask = taskForSelected;
	while (tmpTask) { // loop though list of tasks-for-selected
		if (player_id == tmpTask->player || tmpTask->player == PLAYER::ANY_PLAYER) { // only add tasks for this player or all players
            if ((pAgent->is_sel() && !tmpTask->specific_group) || tmpTask->specific_group == pAgent->get_group_id()) {
                if (!tmpTask->message_only) { // add task
                    // change destination by a little for every group
                    if (tmpTask->type == TASK::MOVE) multiple_agents_move_task(tmpTask, pAgent);
                    // handle units and buildings differently
                    if (pAgent->is_building()) {
                        // add task to building
                        if (tmpTask->makeCurrent) pAgent->add_task_front(tmpTask);
                        else pAgent->add_task(tmpTask);
                    } else {
                        // add task to agent
                        if (tmpTask->makeCurrent) pAgent->clear_tasks();
                        pAgent->add_task(tmpTask);
                    }
                    // send over network
                    if (!tmpTask->specific_group) network_sndtask(tmpTask, pAgent);
                } else { // message
                    // network sync command
                    if (tmpTask->type == TASK::GRPSYNC) {
                        pAgent->sync_network(&tmpTask->net_msg);
                    }
                }
            }
        }
		tmpTask = tmpTask->pNext;
	}
}

void AGENT_HANDLER::multiple_agents_move_task(TASK* t, GROUP* a)
{
	int nx, ny;
	// don't do it for the first agent
	if (!t->num_assigned++) {
		t->radius = a->get_radius();
		return;
	}
	// get group offset
	pather->group_arrange(t->num_assigned-1, nx, ny);
	// apply offset
	t->x = t->groupX + 2.2 * t->radius * nx;
	t->y = t->groupY + 2.2 * t->radius * ny;
	// update task msg
	t->net_msg.msg.task.a = t->x;
	t->net_msg.msg.task.b = t->y;
}

// clear all tasks in the tasks-for-selected queue, shift over the task-for-selected-next-loop queue
void AGENT_HANDLER::shiftOffTasks(void)
{
	TASK* tmp;
	while (taskForSelected) {
		if (taskForSelected->type == TASK::CHECK_WAIT && taskForSelected->sig_flag) setTaskNextLoop(new TASK(TASK::SIGNAL, taskForSelected->player, taskForSelected->id)); // propogate group signals
		tmp = taskForSelected->pNext;
		delete taskForSelected;
		taskForSelected = tmp;
	}
	taskForSelected = taskForSelectedNextLoop;
	taskForSelectedNextLoop = NULL;
}

GROUP* AGENT_HANDLER::create(const char * typeName, int team, int x, int y, int size, int from_x, int from_y, bool disable_fighting)
{
	UNIT_DATA * pUnitData = data->getUnitDataPointer(typeName);
	ANIMATION_DATA * pAniData = data->getAnimationDataPointer(typeName);
	GROUP * pAgent;
	if (pUnitData) {
		pAgent = new GROUP(this, pUnitData, pAniData, NULL, pather, part_sys, attack_data, team, size, x, y, NULL, from_x, from_y, disable_fighting);
		add(pAgent);
		player[team].unit_increment();
	}
	else pAgent = NULL;
	return pAgent;
}

GROUP* AGENT_HANDLER::create_building(const char * typeName, int team, int x, int y, const char * parent)
{
    // round to center of nearest pathfind area
    x = ((int)x/SMALL_AREA_SIZE)*SMALL_AREA_SIZE + SMALL_AREA_SIZE/2;
    y = ((int)y/SMALL_AREA_SIZE)*SMALL_AREA_SIZE + SMALL_AREA_SIZE/2;

    // fail if area is not completely open
    if (!pather->is_open(x, y)) return NULL;

    // get data
    UNIT_DATA * pUnitData = data->getUnitDataPointer(typeName);
    BUILDING_DATA * pBuildData = data->getBuildingDataPointer(typeName);

    // create
	GROUP * pAgent;
	if (pUnitData && pBuildData) {

	    if (pBuildData->is_generic) { // special case for generic buildplots
            team = 0;
            parent= NULL;
        }
		pAgent = new GROUP(this, pUnitData, NULL, pBuildData, pather, part_sys, attack_data, team, 1, x, y, parent);
		add_start(pAgent);
		if (!pBuildData->is_buildsite) player[team].unit_increment();
	}
	else pAgent = NULL;
	return pAgent;
}

void AGENT_HANDLER::add(GROUP* agent)
{
	if (pLast != NULL) pLast->pNext = agent;
	else pFirst = agent;
	agent->pPrevious = pLast;
	pLast = agent;
	length++;
}

void AGENT_HANDLER::add_start(GROUP* agent)
{
	if (pFirst != NULL) pFirst->pPrevious = agent;
	else pLast = agent;
	agent->pNext = pFirst;
	pFirst = agent;
	length++;
}

GROUP * AGENT_HANDLER::find_group(int move_id)
{
    GROUP* pAgent = pFirst;

	// loop through all agents
	while (pAgent != NULL) {
		if (pAgent->get_id() == move_id) return pAgent;
		pAgent = pAgent->pNext;
	}

	return NULL;
}

void AGENT_HANDLER::kill(GROUP* agent)
{
    if (!agent->is_buildsite()) player[agent->get_team()].unit_decrement();
	toKillList.push_back(agent);
}

void AGENT_HANDLER::killList(void) {
	for (int i=0; i<toKillList.size(); i++) {
		killNow(toKillList[i]);
	}
	toKillList.clear();
}

void AGENT_HANDLER::killNow(GROUP* agent)
{
	if (agent != NULL) {
		if (agent != pFirst) agent->pPrevious->pNext = agent->pNext;
		else pFirst = agent->pNext;
		if (agent != pLast) agent->pNext->pPrevious = agent->pPrevious;
		else pLast = agent->pPrevious;
		delete agent;
		length--;
	}
}

void AGENT_HANDLER::killAll(void)
{
	while (length > 0) killNow(pLast);
}

void AGENT_HANDLER::setTasktoSelected(TASK* task)
{
	if (!taskForSelected) taskForSelected = task;
	else {
		TASK* tmpTask = taskForSelected;
		while (tmpTask->pNext) {
			tmpTask = tmpTask->pNext;
		}
		tmpTask->pNext = task;
	}
}

void AGENT_HANDLER::setTaskNextLoop(TASK* task)
{
	if (!taskForSelectedNextLoop) taskForSelectedNextLoop = task;
	else {
		TASK* tmpTask = taskForSelectedNextLoop;
		while (tmpTask->pNext) {
			tmpTask = tmpTask->pNext;
		}
		tmpTask->pNext = task;
	}
}

GROUP * AGENT_HANDLER::find_group(int player, int group_id)
{
    GROUP* pAgent = pFirst;

    // loop through until found
    while (pAgent != NULL) {
        if (pAgent->get_team() == player && pAgent->get_group_id() == group_id)
                    return pAgent;

        // Next agent
		pAgent = pAgent->pNext;
    }

    return NULL;
}

bool AGENT_HANDLER::check_close_behind(GROUP* g, int dist)
{
    int gx = g->get_x();
    int gy = g->get_y();

    GROUP* pAgent = pFirst;

	// loop through all agents
	while (pAgent != NULL) {

        // check if close
        int tx = pAgent->get_x();
        int ty = pAgent->get_y();
        // moving and nearby and in front of
        if (pAgent->get_state() == GROUP::MOVING && (tx-gx)*(tx-gx)+(ty-gy)*(ty-gy) < dist*dist && g->is_near_angle(g->angle_to(tx, ty), 55)) return true;

		// Next agent
		pAgent = pAgent->pNext;
	}

	return false;
}

GROUP* AGENT_HANDLER::get_closest_group(int x, int y, int farthest_dist, int team)
{
    GROUP* pAgent = pFirst;
    GROUP* pClose = NULL;
    int tdist, dist = farthest_dist * farthest_dist;

	// loop through all agents
	while (pAgent != NULL) {

        // check if closer and not building (and team type, if requested)
        if (!pAgent->is_building() && (team == -1 || pAgent->get_team() == team)) {
            int tx = pAgent->get_x();
            int ty = pAgent->get_y();
            tdist = (tx-x)*(tx-x)+(ty-y)*(ty-y);
            if (tdist < dist) {
                pClose = pAgent;
                dist = tdist;
            }
        }

		// Next agent
		pAgent = pAgent->pNext;
	}

	return pClose;
}

GROUP* AGENT_HANDLER::get_closest_nonteamgroup(int x, int y, int farthest_dist, int team)
{
    GROUP* pAgent = pFirst;
    GROUP* pClose = NULL;
    int tdist, dist = farthest_dist * farthest_dist;

	// loop through all agents
	while (pAgent != NULL) {

        // check if closer and not building (and team type, if requested)
        if (pAgent->is_fightable() && pAgent->get_team() != team) {
            int tx = pAgent->get_x();
            int ty = pAgent->get_y();
            tdist = (tx-x)*(tx-x)+(ty-y)*(ty-y);
            if (tdist < dist) {
                pClose = pAgent;
                dist = tdist;
            }
        }

		// Next agent
		pAgent = pAgent->pNext;
	}

	return pClose;
}

GROUP* AGENT_HANDLER::get_building(int team)
{
    GROUP* pAgent = pFirst;

	// loop through all agents
	while (pAgent != NULL) {

        // check if closer and not building (and team type, if requested)
        if (pAgent->is_building() && pAgent->get_team() == team)
            return pAgent;

		// Next agent
		pAgent = pAgent->pNext;
	}

	return NULL;
}

int AGENT_HANDLER::check_winner(void)
{
    int winner = 0;
    int alive = 0;

    for (int i=1; i<player.size(); i++) {
        if (!player[i].is_dead()) {
            alive++;
            winner = i;
        }
    }

    if (alive <= 1) return winner;
    else return 0;
}

void AGENT_HANDLER::send_broadcast(MESSAGE* msg)
{
    if (network)
        network->broadcast((char*)msg, sizeof(*msg));
}

WORLD* AGENT_HANDLER::get_world(void)
{
    return data->world;
}

DATA * AGENT_HANDLER::get_data(void)
{
    return data;
}

PLAYER* AGENT_HANDLER::get_player_pointer(int player_id)
{
    if (player_id >= player.size()) return NULL;
    return &player[player_id];
}

void AGENT_HANDLER::pause(bool set)
{
    is_paused = set;
}

void AGENT_HANDLER::save_agents(FILE * fout)
{
    GROUP* pAgent = pFirst;

	// loop through all agents
	while (pAgent != NULL) {

	    if (pAgent->is_building() || pAgent->is_buildsite()) {
            fprintf(fout, "%s\n 0 %d %d %d %d\n", pAgent->get_type(), (int)pAgent->get_x(), (int)pAgent->get_y(), (int)pAgent->get_first_agent()->get_rotation(), (int)pAgent->get_team());
	    } else {
            fprintf(fout, "%s\n %d %d %d %d %d\n", pAgent->get_type(), pAgent->get_size(), (int)pAgent->get_x(), (int)pAgent->get_y(), (int)pAgent->get_rotation(), (int)pAgent->get_team());

		}

	    // Next agent
		pAgent = pAgent->pNext;
	}

    fputs("end\n", fout);
}

void AGENT_HANDLER::load_agents(FILE * fin)
{
    int tx, ty, tr, tt, nu;
    GROUP * gp;
    char buf[256];

    while (1) {
        do {
            fgets(buf, 256, fin);
        } while (strlen(buf) < 3);
        if (!strncmp(buf, "end", 3) || feof(fin)) return;
        strip_nonprintables(buf);
        fscanf(fin, "%d %d %d %d %d\n", &nu, &tx, &ty, &tr, &tt);
        
		// buildings
		if (!nu) {
			gp = create_building(buf, tt, tx, ty);
        	if (gp) gp->get_first_agent()->set_rotation(tr);
		} else { // units
			gp = create(buf, tt, tx, ty, nu);
			if (gp) {
				TASK* tt = new TASK(TASK::ROTATE, gp->get_team(), tr);
				gp->clear_tasks();
				gp->add_task(tt);
				delete tt;
			}
		}

    }
}

AGENT_HANDLER::AGENT_HANDLER(NETWORK* network, DISPLAY* display, PATHER* pather, DATA* data, PARTICLE_SYSTEM& part_sys, ATTACK_DATA& attack_data, vector<PLAYER>& player_list) : network(network), player(player_list), part_sys(part_sys), attack_data(attack_data), is_paused(false), data(data), display(display), pather(pather)
{
	length = 0;
	pFirst = pLast = NULL;
	taskForSelected = NULL;
	taskForSelectedNextLoop = NULL;
}

AGENT_HANDLER::~AGENT_HANDLER(void)
{
	killAll();
}
