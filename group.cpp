#include "wotwar.h"
#include <string>
#include <limits>

GROUP::GROUP(AGENT_HANDLER * ahp, UNIT_DATA * udp, ANIMATION_DATA * adp, BUILDING_DATA * bdp, PATHER * pp, PARTICLE_SYSTEM& part_sys, ATTACK_DATA& attack_data, int team, int max, double x, double y, const char * parent, double from_x, double from_y, bool disable_fighting) : MOVE(udp, pp, x, y), state(GROUP::NORMAL), agent_handler(ahp), unit_data(udp), ani_data(adp), build_data(bdp), buildplot(NULL), pPrevious(NULL), pNext(NULL), sel(false), attack_type(NULL), resource_counter(0), max_units(max > GROUP_MAXUNITS ? GROUP_MAXUNITS : max), units_in_place(0), find_formation(false), recheck_counter(0), zombie(false), parent_type(parent), building_block(false), interrupted_task(-1), gid(0), nearby_check_counter(0), disable_fighting(disable_fighting)
{
	// init
	set_team(team);
	agent_id = player->new_group_id();
	task_list.init(agent_handler);
	group_radius = 4*udp->radius * sqrt(max_units);

	// attack
	if (unit_data->ranged) attack_type = attack_data.get_type(unit_data->ranged_attack);

	// setup units
	template_agent = new AGENT(this, unit_data, ani_data, build_data, pather, part_sys, attack_type, (int)x, (int)y);
	agent_pos = new VEC[max_units];

	// set initial direction
	if (from_x < 0 || from_y < 0) {
	    from_x = x;
	    from_y = y;
	} else {
	    set_rotation(angle_to(from_x, from_y) + 180);
	    if (max_units == 1) set_loc(from_x, from_y);
	}

    // make units
	add_units(max_units, from_x, from_y);

	// go to formation
	if (!build_data) {
	    // units
        clear_status();
        update_agents_formation();
        point_agents_dest();
	} else if (agents.size()) {
	    // building
	    agents.begin()->sync_to(this);
	    compute_move(200, &build_x, &build_y);
	    pather->add_building_m(x, y); // make area impassible
	    building_block = true;
	}
}

GROUP::~GROUP(void)
{
	select(false); // unselect
	unlink_all(); // unlink tasks
	if (building_block) pather->remove_building_m(x, y); // make area passable

	delete template_agent;
	delete [] agent_pos;
}

void delete_agents(void)
{

}

void GROUP::update(void)
{
    if (!is_building()) update_group();
    else update_building();
}

void GROUP::update_building(void)
{
    // generic stuff
    agents.begin()->update_building();

    // building tasks
    building_tasks();

    // fighting
    if (state == GROUP::RANGED_FIGHTING)
        update_agents_fight();

    // buildplots
    update_buildplots();

    // generate resources
    if (build_data->resources_per_sec) {
        resource_counter++;
        if (resource_counter > RESOURCE_UPDATE_FREQ*GOAL_FPS) {
            resource_counter = 0;
            player->add_resources(build_data->resources_per_sec);
        }
    }

    // check if nearby enemy
	if (nearby_check_counter++ >= GOAL_FPS) {
            nearby_check_counter = 0;
            check_nearbyenemies();
    }

    // die if the building agent has died
    if (agents.begin()->is_zombie()) die();
}

void GROUP::update_buildplots(void)
{
    GROUP* closest;

    if (recheck_counter++ > GOAL_FPS) {
        recheck_counter = 0;
        // check for nearby groups
        if (build_data->is_generic) {
            recheck_counter = 0;
            closest = agent_handler->get_closest_group(x, y, DIST_TO_CLAIM_BUILDSITE);

            // replace
            if (closest) {
                replace_building(false, closest->get_team());
                sync_buildplot(closest->get_team());
            }

        } else if (build_data->is_buildsite && parent_type) { // switch back to generic buildplot
            closest = agent_handler->get_closest_group(x, y, DIST_TO_CLAIM_BUILDSITE, get_team());

            // replace
            if (!closest) die();
        }
    }
}

void GROUP::update_group(void)
{
	// update group
	update_tasks();
	update_self();

	// control agents
	clear_status();
	switch (state) {
		case GROUP::NORMAL :
		case GROUP::IDLE :
			update_agents_idle();
		break;
		case GROUP::UPDATE_FORMATION :
		case GROUP::MOVING :
			if (is_single_unit()) update_agents_idle();
			else update_agents_formation();
		break;
		case GROUP::FIGHTING :
		case GROUP::RANGED_FIGHTING :
			update_agents_fight();
		break;
	}

	// find ideal formation
	if (find_formation) {
	    find_ideal_formation();
	    set_location_to_unit_avg(true);
	    find_formation = false;
	}

	// destroy zombie agents
	update_deaths();
}

void GROUP::update_self(void)
{
    // don't get too far from units
	if ((is_single_unit() && !collision) || (!is_single_unit() && max_unit_dist < 2 * max_units * unit_data->radius)) move((int)get_dest_x(), (int)get_dest_y());
	else if (is_single_unit() && collision) collision_avoid();
	else move_stop();

	num_units = agents.size();

	update_move();
	//if (is_single_unit()) collision_check();
}

void GROUP::update_agents_idle(void)
{
    int i;
	list<AGENT>::iterator p;
	for (p = agents.begin(), i=0; p != agents.end(); p++, i++) {
		// update agent and status
		p->update_normal();
		update_agent(p, i);
	}

	// check if nearby enemy
	if (nearby_check_counter++ >= GOAL_FPS) {
            nearby_check_counter = 0;
            check_nearbyenemies();
    }
}

bool GROUP::check_nearbyenemies(void)
{
	if (disable_fighting) return false;

    GROUP* nearby = agent_handler->get_closest_nonteamgroup(x, y, unit_data->ranged ? unit_data->range : 600, get_team()); // determines line of sight
    if (nearby) {
        warn_attack(nearby);
        sync_attack(nearby->get_team(), nearby->get_group_id());
    }

    return (bool)nearby;
}

void GROUP::update_agents_formation(void)
{
	// local variables
	const int GROUP_MIN_WIDTH = 2;
	int gsize = agents.size();
	int spacing = (int)(unit_data->radius * 3.5);
	if (gsize < 7) group_width = GROUP_MIN_WIDTH + (((gsize-1)/GROUP_MIN_WIDTH <= 1) ? 0 : (gsize-1)/(2*GROUP_MIN_WIDTH)); // calc how many units wide the group should be
	else group_width = 4;
	int w = (int)(group_width * spacing * 0.5 - spacing / 2); // width of group / 2
	int h = (int)(gsize / group_width * spacing * 0.5); // length of group / 2
	int start_lrow = gsize%group_width;

	// precompute
	double theta = rotation * M_PI / 180.;
	double theta_op = theta - M_PI / 2.;
	double cos_theta = cos(theta);
	double cos_theop = cos(theta_op);
    double sin_theta = sin(theta);
	double sin_theop = sin(theta_op);

	// update group radius
	group_radius = w;

	// temp variables
	int j = 0; // unit # in group (counter)
	double extra = 0.; // used to center unfilled rows
	double xpos, ypos, cx, cy;

	// loop through units and set positions
	int i;
	list<AGENT>::iterator p;
	for (p = agents.begin(), i=0; p != agents.end(); p++, i++) {
		// find group position
		if (j == gsize - start_lrow) extra = (group_width - start_lrow) * 0.5; // center the last row (unfilled)
		xpos = j/group_width * spacing - h;
		ypos = ((j%group_width) + extra) * spacing - w;
		j++; // increment position counter
		// find actual position
		cx = x - cos_theta*xpos + cos_theop*ypos;
		cy = y - sin_theta*xpos + sin_theop*ypos;
		// set destination unless it would send agent in the wrong direction
		if (!is_moving() || state == GROUP::UPDATE_FORMATION || is_dest_angle_near(p->angle_to(cx, cy), 60) || p->dist_to(cx, cy) > 10 * p->getRadius()) p->set_dest(cx, cy); // set unit destination

		// update agent and status
		p->update_normal();
		update_agent(p, i);
	}
}

void GROUP::update_agents_fight(void)
{
	int size = agents.size();
	double x_avg = 0;
	double y_avg = 0;
	double x_pos, y_pos;

	// get target and make sure it exists
	GROUP * target = task_list.current()->link;
	if (!target) return;

	// loop through units
	int i;
	list<AGENT>::iterator p;
	for (p = agents.begin(), i=0; p != agents.end(); p++, i++) {
		// find average position
		x_avg += p->get_x();
		y_avg += p->get_y();
		// update agent and status
		if (state == GROUP::FIGHTING) p->update_fight(target);
		else p->update_rangedfight(target);
		update_agent(p, i);
	}

	// set group to average position
	if (!is_building())
        set_loc(x_avg/size, y_avg/size);
}

void GROUP::clear_status(void)
{
	// variables to update every frame
	units_in_place = 0;
	units_collided = 0;
	max_unit_dist = 0;
}

void GROUP::update_agent(list<AGENT>::iterator& p, int index)
{
	int tmp_dist;

	// check distance from group
	if (p->is_dest()) units_in_place++;
	tmp_dist = (int)p->dist_dest();
	if (tmp_dist > max_unit_dist) max_unit_dist = tmp_dist;

	// update position in collision array
	agent_pos[index].x = p->get_x();
    agent_pos[index].y = p->get_y();
    agent_pos[index].flag = p->get_id();
    agent_pos[index].d = p->is_stuckcounter();

	// check if collision at dest
	if (p->is_team_collision()) units_collided++;

	// check if dead
	if (p->is_zombie()) zombies.push_back(p);
}

void GROUP::update_deaths(void)
{
	// kill dead agents
	if (!zombies.empty()) {
		list<list<AGENT>::iterator>::iterator p;
		for (p = zombies.begin(); p != zombies.end(); p++) {
			agents.erase(*p);
			num_units--;
		}
		zombies.clear();
		if (agents.size() == 1) sync_to(&agents.front()); // if only one unit is left in the group, sync the group to it in order to prevent a jump when it starts being synced to the group every update
	}
	// if no more agents, destruct group
	if (agents.empty()) die();
}

void GROUP::building_tasks(void)
{
    BUILDABLE * tbuild;
    GROUP * pg, * target;
    TASK * tt, * dt;
    int tx, ty, tx1, ty1;

    // handle tasks
    switch (task_list.currentType()) {

        case TASK::CREATE_BUILDING :
            state = GROUP::CREATING_BUILDING;
            // initial
            if (!get_first_agent()) return;
            if (!task_list.current()->startCount) {
                task_list.current()->startCount = true;
                get_first_agent()->set_health(1);
            }
            // count
            get_first_agent()->add_health(get_first_agent()->get_data()->maxHealth / (task_list.current()->buildtime * (double)GOAL_FPS));
            if (task_list.current()->generic_counter++ >= GOAL_FPS) {
                task_list.current()->generic_counter = 0;
                task_list.current()->counter--;
            }
            // create building
            if (task_list.current()->counter <= 0) {
                // reselect if selected so that build menu will show up
                if (is_sel()) {
                    select(false);
                    select();
                }
                task_list.endCurrent();
            }
        break;

        case TASK::BUILD_BUILDING :
            state = GROUP::BUILDING;
            // create building
            replace_building(true);
            task_list.endCurrent();
        break;

        case TASK::BUILD :
            state = GROUP::BUILDING;
            // count
            if (task_list.current()->generic_counter++ >= GOAL_FPS) {
                task_list.current()->generic_counter = 0;
                task_list.current()->counter--;
            }
            // build group
            if (task_list.current()->counter <= 0) {
                tbuild = &build_data->buildables[task_list.current()->build_id];
                compute_move(SMALL_AREA_SIZE * 0.5, &tx, &ty, angle_to(build_x, build_y));
                compute_move(SMALL_AREA_SIZE * 0.7, &tx1, &ty1, angle_to(build_x, build_y));
                pg = agent_handler->create(tbuild->name, get_team(), tx1, ty1, tbuild->quantity,  tx, ty);
                if (pg) {
                    tt = new TASK(TASK::MOVE, get_team(), build_x, build_y, true);
                    pg->add_task(tt);
                    delete tt;
                } else {
                    puts("warning: building attempted to create an undefined unit!");
                }
                task_list.endCurrent();
            }
        break;

        case TASK::UNBUILD :
            dt = NULL;
            tt = task_list.pFirst;
            while (tt) {
                if (tt->type == TASK::BUILD && tt->build_id == task_list.current()->build_id) dt = tt;
                tt = tt->pNext;
            }
            if (dt) {
                dt->type = TASK::SKIP;
                player->add_resources(build_data->buildables[dt->build_id].cost);
            }
            task_list.endCurrent();
        break;

        case TASK::MOVE :
            // set build point
            build_x = task_list.current()->x;
            build_y = task_list.current()->y;
            task_list.endCurrent();
        break;

        case TASK::FIGHT : // Fight
			target = task_list.current()->link;
			if (!target || !target->is_fightable()) { // the task will automagicly unlink when the enemy dies
				task_list.endCurrent();
				state = GROUP::IDLE;
				break;
			} else {
			    // fighting state
				if (unit_data->ranged && dist_to(target->get_x(), target->get_y()) < get_radius() + unit_data->range) { // ranged fighting
                    state = GROUP::RANGED_FIGHTING;
				} else { // end fight
					task_list.endCurrent();
				}
			}
		break;

        case TASK::NONE :
            state = GROUP::IDLE;
        break;

        default:
            task_list.endCurrent();
        break;
    }
}

void GROUP::update_tasks(void)
{
	// local variables
	const int step_size = 100;
	bool new_task_flag = is_new_task(); // checks if we have a new task
	GROUP* target = NULL;

	// check if interrupted task
    if (new_task_flag && task_list.current() && task_list.current()->id == interrupted_task)
        interrupted_task = -1;

	// tmp variables
	GROUP* tg;
	TASK * tt;
	int tx, ty;

	// handle tasks
	switch (task_list.currentType()) {

		case TASK::NONE :
			state = GROUP::IDLE;
			move_stop();
		break;

		case TASK::WAIT_COLLISION :
            state = GROUP::IDLE;
            if (!recheck_counter) ;
            if (recheck_counter >= 20) {
                recheck_counter = 0;
                if (!agent_handler->check_close_behind(this, 50)) task_list.endCurrent();
            } else recheck_counter++;
		break;

		case TASK::MOVE : // Move to area
            // re-check stuff (computationally intensive checks)
            if ((check_x-x)*(check_x-x)+(check_y-y)*(check_y-y) > step_size*step_size) {
                check_x = (int)x;
                check_y = (int)y;
                // check if target is nearby
                GROUP * target = task_list.find_next_attack();
                if (target) {
                    int dist = dist_to(target->get_x(), target->get_y());
                    if ((unit_data->melee && dist < get_radius() + target->get_radius()) || (unit_data->ranged && dist < get_radius() + unit_data->range)) {
                        task_list.drop_until_attack();
                        break;
                    }
                }
            }
            // go to enemy move
			if (task_list.current()->linked_flag) {
				target = task_list.current()->link;
				if (!target || dist_to(target->get_x(), target->get_y()) < get_radius() + target->get_radius()) { // end task if target is nearby or dead
					task_list.endCurrent();
					break;
				} else {
				    state = GROUP::MOVING;
				    set_dest(target->get_x(), target->get_y());
				}
			} else { // normal move
				state = GROUP::MOVING;
				set_dest(task_list.current()->x, task_list.current()->y);
				if (new_task_flag) {
				    // don't try to move off the map
				    task_list.current()->fit_to_map(agent_handler->get_world());
				    // don't move on top of other groups
				    tg = agent_handler->get_agent_within(get_dest_x(), get_dest_y(), 25, this);
				    if (tg/* && task_list.current()->is_final*/) {
				        if (tg->get_team() == get_team() && !tg->is_moving() && tg->get_tasklist()->currentType() == TASK::NONE) {
				            tg->compute_move(1.75 * max(get_radius(), tg->get_radius()), &tx, &ty);
				            tt = new TASK(TASK::MOVE, get_team(), tx, ty, true);
				            tg->clear_tasks();
                            tg->add_task(tt);
                            delete tt;
				        }
				    }
				}
			}
			// general movement stuff
			if (new_task_flag && !is_single_unit()) {
                    // if pointing too far off, jump to direction
					if (abs((int)rotation_to_dest()) > 120) {
						set_rotation(get_angle_dest()); // if pointing too far in the wrong direction just point to dest
						set_location_to_unit_avg();
						flip_formation();
					}
                    // jump ahead a little
					if (units_in_place >= agents.size()/2) shift(unit_data->radius, get_angle_dest());
			}
			// generic stuff
			if (new_task_flag && gen_pathfind(task_list.current()->pathfind_level)) set_dest(x, y); // gen_pathfind will end the task if there is a pathfinding problem
			if (is_dest()) task_list.endCurrent(); // end task
		break;

		case TASK::ROTATE : // Rotate
			state = GROUP::MOVING;
			rotate_towards(task_list.current()->theta);
			move_stop();
			if (new_task_flag) units_to_group_rotation();
			if (is_angle(task_list.current()->theta)) task_list.endCurrent();
		break;

		case TASK::FIGHT : // Fight
			if (disable_fighting) { // disabled
				task_list.endCurrent();
				state = GROUP::NORMAL;
				break;
			}

			target = task_list.current()->link;
			if (!target || !target->is_fightable()) { // the task will automagicly unlink when the enemy dies
				task_list.endCurrent();
				state = GROUP::IDLE;
				if (!check_nearbyenemies()) {
                    //generate_ideal_formation();
                    revert_interrupted();
                    state = GROUP::UPDATE_FORMATION;
				}
				set_location_to_unit_avg();
				// the host sends a network sync
				if (player->is_host()) sync_grpsync();
				break;
			} else {
				// add to opponent's attacklist
				if (!task_list.current()->attacklisted) {
					task_list.current()->attacklisted = this;
					target->add_attacker(this);
				}
			    // fighting state
				if (unit_data->melee && dist_to(target->get_x(), target->get_y()) < get_radius() + target->get_radius() + 25) { // melee fighting
					state = GROUP::FIGHTING;
					if (!is_single_unit()) set_rotation(angle_to(target->get_x(), target->get_y()));
					move_stop();
                } else if (unit_data->ranged && dist_to(target->get_x(), target->get_y()) < get_radius() + unit_data->range) { // ranged fighting
                    state = GROUP::RANGED_FIGHTING;
                    if (!is_single_unit()) set_rotation(angle_to(target->get_x(), target->get_y()));
                    move_stop();
				} else { // go to fight
					state = GROUP::NORMAL;
					task_list.add_front(new TASK(TASK::MOVE, team_id, target));
					break;
				}
			}
		break;

		default:
            task_list.endCurrent();
		break;
	}
}

int GROUP::gen_pathfind(int level)
{
	TASK * t = NULL;

	// do a switch-case for specific pathfinding depending on the level requested, not bothering right now since there is only one level of pathfinding so a simple "if (level == 0)" suffices
	if (!level) t = pather->get_path_closest((int)get_x(), (int)get_y(), (int)get_dest_x(), (int)get_dest_y());
	else return 0;

	// add new tasks to start of task_list
	if (t) {
		task_list.current()->pathfind_level++;
		task_list.insertList(NULL, t); // add pathfind tasks
	}
	return pather->mapsearchnode_error;
}

void GROUP::revert_interrupted(void)
{
    // remove dead tasks
    if (interrupted_task == -1) return;

    // revert to interrupted task
    while (task_list.current() && task_list.current()->id != interrupted_task)
        task_list.endCurrent();

    interrupted_task = -1;

    if (task_list.currentType() != TASK::MOVE || (task_list.currentType() == TASK::MOVE && task_list.current()->is_final)) return;

    // delete pathfind tasts
    while (task_list.currentType() == TASK::MOVE && !task_list.current()->is_final)
        task_list.endCurrent();

    // repathfind
    if (task_list.currentType() == TASK::MOVE)
        task_list.current()->pathfind_level = 0;
}

bool GROUP::is_new_task(void)
{
	static unsigned int cur_task_id = -1;
	if (task_list.current() && cur_task_id != task_list.current()->id) {
		cur_task_id = task_list.current()->id;
		return true;
	} else {
		return false;
	}
}

void GROUP::clear_tasks(void)
{
	task_list.dropAll();
}

void GROUP::add_task(TASK * tp)
{
	TASK* tmpTask = new TASK;
	*tmpTask = *tp; // copy task
	tmpTask->relink(); // need to relink since it has been copied
	this->task_list.add(tmpTask); // add to task list
}

void GROUP::add_task_front(TASK * tp)
{
	TASK* tmpTask = new TASK;
	*tmpTask = *tp; // copy task
	tmpTask->relink(); // need to relink since it has been copied
	this->task_list.add_front(tmpTask); // add to task list
}

TASK::TASK_TYPE GROUP::task_type(void)
{
	return task_list.currentType();
}

void GROUP::add_units(int n, double x1, double y1)
{
    // default to group position
    if (x1 < 0 || y1 < 0) {
        x1 = x;
        y1 = y;
    }

    // add
	list<AGENT>::iterator p;
	for (; n; n--) {
		if (agents.size() >= max_units) break;
		num_units++;
		agents.push_back(*template_agent); // add to agent list
		p = agents.end();
		p--;
		agents_draw_order.push_back(p); // add to draw order
		// should put these three lines in an agent function
		p->set_id(gid);
		p->set_sort_id(gid++);
		p->set_collide_node(&agents.back());
		p->set_loc(x1, y1);
		if (x1 != x || y1 != y) p->set_rotation(p->angle_to(x, y));
	}
	sort_units();
}

void GROUP::kill_unit(list<AGENT>::iterator& up)
{
	zombies.push_back(up);
}

void GROUP::generate_ideal_formation(void)
{
    find_formation = true;
}

void GROUP::find_ideal_formation(void) // an approximation, actual ideal would be too involved
{
    VEC pos[num_units];
	list<AGENT>::iterator p;
	// generate list of positions
	int i=0;
	double avg_x=0, avg_y=0;
	for (p = agents.begin(); p != agents.end(); p++) {
	    pos[i].x = p->get_dest_x();
	    pos[i].y = p->get_dest_y();
	    pos[i].flag = false;
	    avg_x += p->get_x();
	    avg_y += p->get_y();
	    i++;
	}
	avg_x = avg_x/agents.size() - get_x();
	avg_y = avg_y/agents.size() - get_y();
	// find ideal positions
	for (p = agents.begin(); p != agents.end(); p++) {
        int closest = 0;
        double min_dist = __DBL_MAX__;
        double t_dist, t_x, t_y;
        for (int i=0; i<num_units; i++) { // find closest unoccupied position
            if (pos[i].flag) continue;
            t_x = pos[i].x - p->get_x() - avg_x;
            t_y = pos[i].y - p->get_y() - avg_y;
            t_dist = t_x*t_x + t_y*t_y;
            if (t_dist < min_dist) {
                min_dist = t_dist;
                closest = i;
            }
        }
        pos[closest].flag = true;
        p->set_sort_id(closest);
	}
	// sort the agents into position
	sort_units();
}

void GROUP::flip_formation(void)
{
    bool odd = agents.size()%group_width;
    // flip
    int sort_id = 0;
    int i = -1;
    list<AGENT>::iterator p, q = agents.end();
    for (p = agents.begin(); p != agents.end(); p++) {
        i++;
        if (odd && i%group_width >= (group_width - odd)) p->set_sort_id(sort_id - group_width); // pull some tricksyness with odd sized groups
        else p->set_sort_id(sort_id); // swap spots
        sort_id--;
    }
    sort_units();
}

GROUP* GROUP::get_opponent()
{
	if (task_list.currentType() != TASK::FIGHT) return NULL;

	return task_list.current()->link;
}

bool GROUP::check_insidegroup_collision(double x, double y, unsigned int id, int& xhit, int& yhit)
{
    double radius_s = 2.5*template_agent->getRadius();
    radius_s = radius_s*radius_s;
    double x_tmp, y_tmp;

    for (int i=0; i<agents.size(); i++) {
        if (id == agent_pos[i].flag) continue;
        x_tmp = x - agent_pos[i].x;
        y_tmp = y - agent_pos[i].y;
        if (x_tmp*x_tmp + y_tmp*y_tmp < radius_s) {
			xhit = agent_pos[i].x;
			yhit = agent_pos[i].y;
			return true;
		}
    }
    return false;
}

int GROUP::check_insidegroup_collision3(double x, double y, double xprev, double yprev, double speed, unsigned int id, int& xhit, int& yhit)
{
    double radius_s = 1.5*template_agent->getRadius();
    radius_s = radius_s*radius_s;
    double x_tmp, y_tmp;

	double xup = (x > xprev) ? speed : -speed;
	double yup = (y > yprev) ? speed : -speed;

    for (int i=0; i<agents.size(); i++) {
        if (id == agent_pos[i].flag || (agent_pos[i].d && id > agent_pos[i].flag)) continue;
        
		// check free movement
		x_tmp = x - agent_pos[i].x;
        y_tmp = y - agent_pos[i].y;
        if (x_tmp*x_tmp + y_tmp*y_tmp > radius_s) continue;

		xhit = agent_pos[i].x;
		yhit = agent_pos[i].y;
		
		// check free x movement
		x_tmp = x - agent_pos[i].x + xup;
        y_tmp = yprev - agent_pos[i].y;
        if (x_tmp*x_tmp + y_tmp*y_tmp > radius_s) return 2;
		
		// check free y movement
		x_tmp = xprev - agent_pos[i].x;
        y_tmp = y - agent_pos[i].y + yup;
        if (x_tmp*x_tmp + y_tmp*y_tmp > radius_s) return 3;

		return 1;
    }
    return 0;
}

int GROUP::check_insidegroup_collision2(double x, double y, double xprev, double yprev, unsigned int id)
{
    double radius_s = 1.5*template_agent->getRadius();
    radius_s = radius_s*radius_s;
    double x_tmp, y_tmp;

    for (int i=0; i<agents.size(); i++) {
        if (id == agent_pos[i].flag) continue;
        
		// check free movement
		x_tmp = x - agent_pos[i].x;
        y_tmp = y - agent_pos[i].y;
        if (x_tmp*x_tmp + y_tmp*y_tmp > radius_s) continue;
		
		// check free x movement
		x_tmp = x - agent_pos[i].x;
        y_tmp = yprev - agent_pos[i].y;
        if (x_tmp*x_tmp + y_tmp*y_tmp > radius_s) return 2;
		
		// check free y movement
		x_tmp = xprev - agent_pos[i].x;
        y_tmp = y - agent_pos[i].y;
        if (x_tmp*x_tmp + y_tmp*y_tmp > radius_s) return 3;

		return 1;
    }
    return 0;
}

void GROUP::sort_units(void)
{
	agents.sort(sorter);
}

bool GROUP::sorter(AGENT& a1, AGENT& a2)
{
	return (a1.get_sort_id() < a2.get_sort_id());
}

bool GROUP::is_building(void)
{
    return (bool)build_data;
}

bool GROUP::is_buildsite(void)
{
    return (bool)(build_data && (build_data->is_generic || build_data->is_buildsite));
}

bool GROUP::is_fightable(void)
{
    return (!build_data || !is_buildsite());
}

void GROUP::sync_network(MESSAGE* msg)
{
    if (msg->type != MSGTYPE_GRPSYNC) return; // sanity check

    MESSAGE_GRPSYNC* tmsg = &msg->msg.gsync;
    list<AGENT>::iterator p;
    int i = 0;
    GROUP* enemy = NULL;

    switch (tmsg->grpsync_type) {
        case GRPSYNC_GROUP:
            // increase number of units
            if (tmsg->count > agents.size())
                add_units(tmsg->count - agents.size());

            // sync each unit
            for (p = agents.begin(); p != agents.end(); p++, i++) {
                if (i < tmsg->count) p->set_health(tmsg->data[i]);
                else p->set_health(0);
            }

            // sync group position
            if (dist_to(tmsg->x, tmsg->y) > NETWORK_ALLOWABLE_GROUPERROR) {
                set_loc(tmsg->x, tmsg->y);
                if (is_single_unit())
                    get_first_agent()->set_dest(tmsg->x, tmsg->y);
            }

            break;

        case GRPSYNC_TARGET:
            if (tmsg->pid == get_team()) break; // sanity check

            // find target
            enemy = agent_handler->find_group(tmsg->pid, tmsg->target_id);
            if (!enemy) break;

            // set target
            warn_attack(enemy);

            break;

        case GRPSYNC_BUILDING:
            if (!is_building()) break; // sanity check

            // a group got close to this as a buildplot
            if (!tmsg->count && tmsg->pid != get_team() && build_data->is_generic) {
                replace_building(false, tmsg->pid);
            // time to die
            } else if (!tmsg->count && !build_data->is_generic) {
                die();
            }

            break;
    }
}

void GROUP::sync_initmsg(MESSAGE* msg)
{
    msg->size = sizeof(*msg);
    msg->type = MSGTYPE_GRPSYNC;
    msg->player_id = get_team();
    msg->msg.gsync.group_id = get_group_id();
    msg->msg.gsync.type = TASK::GRPSYNC;
    msg->msg.gsync.count = agents.size();
}

void GROUP::sync_grpsync(void)
{
    MESSAGE tmsg;
    sync_initmsg(&tmsg);

    tmsg.msg.gsync.grpsync_type = GRPSYNC_GROUP;
    tmsg.msg.gsync.x = get_x();
    tmsg.msg.gsync.y = get_y();

    list<AGENT>::iterator p;
    int i = 0;
    for (p = agents.begin(); p != agents.end(); p++, i++) {
        tmsg.msg.gsync.data[i] = p->get_health();
    }

    agent_handler->send_broadcast(&tmsg);
}

void GROUP::sync_attack(int player, int target_id)
{
    MESSAGE tmsg;
    sync_initmsg(&tmsg);

    tmsg.msg.gsync.grpsync_type = GRPSYNC_TARGET;
    tmsg.msg.gsync.pid = player;
    tmsg.msg.gsync.target_id = target_id;

    agent_handler->send_broadcast(&tmsg);
}

void GROUP::sync_buildplot(int build_player)
{
    MESSAGE tmsg;
    sync_initmsg(&tmsg);

    tmsg.msg.gsync.grpsync_type = GRPSYNC_BUILDING;
    tmsg.msg.gsync.count = 0;
    tmsg.msg.gsync.pid = build_player;

    agent_handler->send_broadcast(&tmsg);
}

void GROUP::sync_die(void)
{
    sync_grpsync();
}

void GROUP::replace_building(bool create_new, int team)
{
    if (!is_building()) return;

    if (team == -1) team = get_team();
    string stmp;

    // clear passibility
    buildplot = build_data->name;
    if (building_block) pather->remove_building_m(x, y); // make area passible
    building_block = false;

    // new building name
    const char * b_name = NULL;

    if (create_new) {
        b_name = build_data->buildables[task_list.current()->build_id].name;
    } else if (build_data->is_generic) {
        stmp = agent_handler->get_data()->get_civname(player->get_faction());
        stmp += " ";
        stmp += build_data->name;
        b_name = stmp.c_str();
    } else {
        b_name = parent_type;
    }

    // create replacement
    GROUP* gp = agent_handler->create_building(b_name, team, x, y, (create_new || build_data->is_generic) ? (parent_type ? parent_type : unit_data->name) : NULL);
    die();

    if (!gp) {
        puts("warning: attempted to create an undefined building!");
        return;
    }

    gp->set_rotation(get_rotation());

    // don't set build task or select if we are recreating a parent building or a team buildsite
    if (!create_new) return;

    // set building task
    TASK* tt = new TASK(TASK::CREATE_BUILDING, get_team(), 0, task_list.current()->counter);
    gp->add_task(tt);
    delete tt;

    // reselect if selected
    if (is_sel()) {
        select(false);
        gp->select();
    }
}

void GROUP::drawToMiniMap(DISPLAY* display, int x, int y, double scale)
{
	list<AGENT>::iterator p;
	for (p = agents.begin(); p != agents.end(); p++) {
		p->drawToMiniMap(display, x, y, scale);
	}
}

void GROUP::select(bool s)
{
	if (s != sel) {
		// add to player selection
		if (s) player->add_selected(this);
		else player->remove_selected(this);
		// set selected
		sel = s;
	}
}

bool GROUP::is_sel(void)
{
	return sel;
}

bool GROUP::is_single_unit(void)
{
	return (num_units == 1);
}

void GROUP::set_team(int team)
{
	team_id = team;
	player = agent_handler->get_player(team_id);
	team_color = player->get_color();
}

int GROUP::get_team(void)
{
	return team_id;
}

int GROUP::get_team_color(void)
{
	return team_color;
}

int GROUP::get_size(void)
{
    return agents.size();
}

int GROUP::get_radius(void)
{
	return group_radius;
}

GROUP::STATE GROUP::get_state(void)
{
	return state;
}

void GROUP::units_to_group_rotation(void)
{
	list<AGENT>::iterator p;
	for (p = agents.begin(); p != agents.end(); p++) {
		p->start_rotate_to_group();
	}
}

void GROUP::draw(DISPLAY* disp)
{
	list<AGENT>::iterator p;
	for (p = agents.begin(); p != agents.end(); p++) {
		if (p->isOnScreen(disp)) {
			p->draw(disp);
		}
	}
}

void GROUP::draw_selected(DISPLAY* disp)
{
	list<AGENT>::iterator p;
	for (p = agents.begin(); p != agents.end(); p++) {
		if (p->isOnScreen(disp)) p->draw_selected(disp);
	}
}

void GROUP::draw_task_path(DISPLAY* display) {
	int x1 = (int) x;
	int y1 = (int) y;
	TASK* link = task_list.current();
	while (link) {
		if (link->type == TASK::MOVE) {
			if ((x1 < display->getWorldX() + display->getWidth() && link->x > display->getWorldX()) || (link->x < display->getWorldX() + display->getWidth() && x1 > display->getWorldX()) || (y1 < display->getWorldY() + display->getHeight() && link->y > display->getWorldY()) || (link->y < display->getWorldY() + display->getHeight() && y1 > display->getWorldY())) fastline(display->buffer, display->toScreenX(x1), display->toScreenY(y1), display->toScreenX(link->x), display->toScreenY(link->y), makecol(200, 200, 200));
			x1 = link->x;
			y1 = link->y;
		}
		link = link->pNext;
	}
}

void GROUP::draw_path_minimap(DISPLAY* display, int x, int y, double scale)
{
	int x1 = (int) this->x;
	int y1 = (int) this->y;
	TASK* link = task_list.current();
	while (link) {
		if (link->type == TASK::MOVE) {
			fastline(display->buffer, x+(int)(x1*scale), y+(int)(y1*scale), x+(int)(link->x*scale), y+(int)(link->y*scale), makecol(200, 200, 200));
			x1 = link->x;
			y1 = link->y;
		}
		link = link->pNext;
	}
}

void GROUP::draw_minmap_buildpoint(DISPLAY* display, int x, int y, double scale)
{
    int toX = x + (int)(build_x * scale);
	int toY = y + (int)(build_y * scale);
	PUTPIXELS_X(display->buffer, toX, toY, makecol(255, 255, 255));
}

void GROUP::point_agents_dest(void)
{
	list<AGENT>::iterator p;
	for (p = agents.begin(); p != agents.end(); p++) {
		p->set_rotation(p->get_angle_dest());
	}
}

AGENT * GROUP::find_closest(double nx, double ny)
{
	double tmpd;
	double cd = __DBL_MAX__;
	AGENT * cl = NULL;

	list<AGENT>::iterator p;
	for (p = agents.begin(); p != agents.end(); p++) {
		tmpd = p->dist_to(nx, ny);
		if (tmpd < cd) {
			cd = tmpd;
			cl = &(*p);
		}
	}

	return cl;
}

bool GROUP::is_near_agent(int x, int y)
{
	list<AGENT>::iterator p;
	for (p = agents.begin(); p != agents.end(); p++) {
		if (p->is_near(x, y)) return true;
	}
	return false;
}

int GROUP::dist_to_agent(int x, int y)
{
	int closest = numeric_limits<int>::max();
	int dist;

	list<AGENT>::iterator p;
	for (p = agents.begin(); p != agents.end(); p++) {
		dist = p->dist_to(x, y) - p->getRadius();
		if (dist < closest) closest = (dist < 0) ? 0 : dist;
	}

	return closest;
}

/*
// AGENT * find_random_closest(double nx, double ny, int num);
AGENT * GROUP::find_random_closest(double nx, double ny, int num) // A LITTLE BUGGY!!!
{
    if (num > agents.size() || num <= 1) return find_random_agent();
    double tmpd;
	double cd[num];
	AGENT * cl[num];
	for (int k=0; k<num; k++) {
	    cd[k] = __DBL_MAX__;
	    cl[k] = NULL;
	}

    list<AGENT>::iterator p;
	for (p = agents.begin(); p != agents.end(); p++) {
		tmpd = p->dist_to(nx, ny);
		if (tmpd < cd[num-1]) {
		    int i;
			for (i=num-1; i--; tmpd < cd[i]) {
			     if (i < num-1) {
			         cd[i+1] = cd[i];
			         cl[i+1] = cl[i];
			     }
			     if (!i) break;
			}
            cd[i] = tmpd;
            cl[i] = &(*p);
		}
	}
    //for (int k=0; k<num; k++) fprintf(stderr, "%d ", cd[k]);
    //fprintf(stderr, "\n");
	return cl[rand()%num];
}
*/

void GROUP::add_attacker(GROUP* g)
{
	attackers.push_back(g);
}

void GROUP::remove_attacker(GROUP* g)
{
	attackers.remove(g);
}

list<GROUP*>* GROUP::get_attackers()
{
	return &attackers;
}

AGENT * GROUP::get_first_agent(void)
{
    return &(*agents.begin());
}

AGENT * GROUP::find_random_agent(int seed)
{
    if (!agents.size()) return NULL;
    if (seed >= agents.size()) seed = seed%agents.size();

    if (agents.size() == 1) return &agents.front();
    list<AGENT>::iterator p = agents.begin();
    for (int i = 0; i < seed; i++) p++;
    return &(*p);
}

void GROUP::compute_average_location(int& xs, int& ys)
{
	double xavg = 0.;
	double yavg = 0.;

	list<AGENT>::iterator p;
	for (p = agents.begin(); p != agents.end(); p++) {
		xavg += p->get_x();
		yavg += p->get_y();
	}

	xs = (int)(xavg / agents.size());
	ys = (int)(yavg / agents.size());
}

void GROUP::set_location_to_unit_avg(bool also_dest)
{
	int xavg, yavg;
	compute_average_location(xavg, yavg);
	set_loc(xavg, yavg);
	if (also_dest) set_dest(xavg, yavg);
}

double GROUP::dist_from_agent_avg(void)
{
    int xavg, yavg;
	compute_average_location(xavg, yavg);
	return sqrt((xavg-x)*(xavg-x) + (yavg-y)*(yavg-y));
}

void GROUP::warn_attack(GROUP * gp)
{
	if ((state == GROUP::IDLE || state == GROUP::MOVING) && (!task_list.last() || task_list.last()->link != gp)) {
        if (interrupted_task == - 1 && task_list.current()) interrupted_task = task_list.current()->id;
        task_list.add_front(new TASK(TASK::FIGHT, team_id, gp));
	}
}

vector<BUILDABLE>* GROUP::get_buildable(void)
{
    if (!build_data) return NULL;
    return &build_data->buildables;
}

void GROUP::die(void)
{
	if (!zombie) {
	    zombie = true;
	    agent_handler->kill(this);
	    if (!build_data || !build_data->is_generic) sync_die();
	    if (parent_type) replace_building(false);
	}
}

TASKLIST * GROUP::get_tasklist(void)
{
    return &task_list;
}

void GROUP::link_task(TASK * t)
{
	linked_tasks.push_back(t);
}

void GROUP::unlink_task(TASK * t)
{
	t->unlink();
	linked_tasks.remove(t);
}

void GROUP::unlink_all(void)
{
	list<TASK*>::iterator i;
	for (i = linked_tasks.begin(); i != linked_tasks.end(); i++) {
		(*i)->unlink();
	}
	linked_tasks.clear();
}

AGENT * GROUP::find_unit(int unit_id)
{
    list<AGENT>::iterator p;
	for (p = agents.begin(); p != agents.end(); p++) {
		if (p->get_id() == unit_id) return &(*p);
	}

	return NULL;
}

AGENT * GROUP::find_nextunit(int unit_id)
{
	bool hit = false;
	int c = 2*get_size()/3;

    list<AGENT>::iterator p = agents.begin();
	for (; ; p++) {
		if (p == agents.end()) {
			if (hit) p = agents.begin();
			else break;
		}
		
		if (hit) c--;
		if (c == 0) return &(*p);
		if (p->get_id() == unit_id) hit = true;
	}

	return NULL;
}

int GROUP::get_group_id(void)
{
    return agent_id;
}

const char * GROUP::get_type(void)
{
    return unit_data->name;
}

const char * GROUP::get_parenttype(void)
{
    return parent_type;
}
