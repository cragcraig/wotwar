#include "wotwar.h"

void AGENT::update(void)
{
	// move
	if ((!group->is_single_unit() || state == AGENT::READY_FIGHT) && !group->is_building()) {
        update_move();
        collision_check();
	}

	// generic unit status
	check_health();
	if (!has_not_moved()) clear_stuckcounter();
	if (!build_data) updateAnimation();
}

void AGENT::update_building(void)
{
    check_health();
}

void AGENT::update_normal(void)
{
	int tx, ty;

	state = AGENT::NORMAL;

	// Logic
	if (!is_dest() && !at_unreachable_dest()) {
		// move faster if out of position
		if (within_dist_dest(unitData->radius * 2)) move_mult(1.0);
		else if (within_dist_dest(unitData->radius * 4)) move_mult(1.1);
		else move_mult(1.8);
		// animation
		setAnimation(ANIMATION_DATA::MOVE, true);
	} else {
		// clear last swap memory
		last_swap = -1;
		// don't move
		if (!is_dest_angle() && group->task_type() == TASK::MOVE) rotate_towards((int)group->get_angle_dest()); // rotate towards target
		else if (rotate_to_group) {
			if (!is_angle(group->get_rotation())) rotate_towards((int)group->get_rotation()); // rotate to group direction
			else end_rotate_to_group();
		}
		move_stop();
		if (!is_moving()) setAnimation(ANIMATION_DATA::STAND, true);
	}

	// Movement
	if (group->is_single_unit()) sync_to(group);
	else {
		swap_count--;
		if (!collision) move((int)get_dest_x(), (int)get_dest_y(), !group->is_moving()); // no collision
		else if (!is_dest() && (!agent_collided || agent_collided->getGroup() != getGroup()) && dist_dest() < group->get_radius()) {
		    if (agent_collided) shorten_dest(2 * getRadius());
		    else {
		        set_dest(group->get_x(), group->get_y());
		        shorten_dest(100);
		    }
		} else if (agent_collided && !is_dest() && group->is_dest_angle() && agent_collided->is_near_dest_angle(35) && agent_collided->is_near_dest() && is_near_angle(agent_collided->get_rotation(), 35) && agent_collided->getGroup() == getGroup() && ((!is_last_swap(agent_collided->get_sort_id()) && !agent_collided->is_last_swap(sort_id)) || swap_count < 0)) { // swap group positions
			swap_positions(agent_collided); // in same group, do it
		}
		else collision_avoid(); // collision
	}

	// generic update
	update();
}

AGENT * AGENT::find_target(GROUP * target_group)
{
    // get target
	AGENT * t = target.get_link();

	// find a target agent
	if (!t || !fight_timer) {
		if (!fight_timer) fight_timer = 20;
		//int rand_off = getRadius() * ((rand()%300 - 150)/100.);
		t = target_group->find_closest(x, y); // find closest agent
		target.switch_link(t);
		clear_stuckcounter();
	}

	return t;
}

AGENT * AGENT::find_target_random(GROUP * target_group)
{
    // get target
	AGENT * t = target.get_link();

	// find a target agent
	if (!t || !fight_timer) {
		if (!fight_timer) fight_timer = 20;
		int rad = target_group->get_radius();
		t = target_group->find_random_agent(sort_id);
		target.switch_link(t);
	}

	return t;
}

void AGENT::update_fight(GROUP* target_group)
{
	// get target
	AGENT * t = find_target(target_group); //group->is_single_unit() ? find_target(target_group) : find_target_random(target_group);
	if (!t) return; // didn't find one

	// fight
	move_mult(collision ? 3.0 : 1.0);
	set_dest(t->get_x(), t->get_y());
	if (dist_dest() > (t->getRadius() + getRadius()) * 2) { // move to target
		fight_timer--;
		if (!collision) move((int)get_dest_x(), (int)get_dest_y(), true); // move
		else collision_avoid(); // collision avoid
		state = AGENT::READY_FIGHT;
		setAnimation(is_stuckcounter() ? ANIMATION_DATA::STAND : ANIMATION_DATA::MOVE, true);
	} else { // at target
		move_stop();
		state = AGENT::FIGHTING;
		if (is_near_dest_angle(45)) { // deal damage
			set_dest(x, y);
			t->hurt_by(unitData->damage, this);
			setAnimation(ANIMATION_DATA::ATTACK, true);
		} else { // face target
			fight_timer--;
			rotate_towards((int)get_angle_dest());
			if (!is_near_dest_angle(90)) setAnimation(ANIMATION_DATA::STAND, true);
		}
	}

	// single units sync group position with their own
	if (group->is_single_unit()) group->sync_to(this);

	// update gemeric stuff
	update();
}

void AGENT::update_rangedfight(GROUP* target_group)
{
	// get target
	AGENT * t = find_target_random(target_group);
	if (!t) return; // didn't find one

	// fight
	move_mult(collision ? 3.0 : 1.0);
	set_dest(t->get_x(), t->get_y());
	if (dist_dest() > unitData->range - group->get_radius()/2) { // move to target
		fight_timer--;
		if (!collision) move((int)get_dest_x(), (int)get_dest_y(), true); // move
		else collision_avoid(); // collision avoid
		state = AGENT::READY_FIGHT;
		setAnimation(is_stuckcounter() ? ANIMATION_DATA::STAND : ANIMATION_DATA::MOVE, true);
	} else { // close enough to target
		move_stop();
		state = AGENT::FIGHTING;
		double angle = rotation * M_PI / 180.;
		double angle_target = angle_to(t->get_x() - unitData->r_horiz_offset*cos(angle + M_PI/2), t->get_y() - unitData->r_horiz_offset*sin(angle + M_PI/2));
		if (is_near_angle(angle_target, 15) || group->is_building()) { // deal damage
			// create projectile
			if (frame_num() == unitData->rattack_frame) {
			    if (is_first_frame_display()) {
			        double t_angle = angle_target*M_PI/180;
			        int plife = (int)(dist_to(t->get_x(), t->get_y()) - getRadius() - 2*t->getRadius());
                    part_sys.add(attack_type->particle, x + getRadius()*cos(t_angle) + unitData->r_horiz_offset*cos(t_angle + M_PI/2), y + getRadius()*sin(t_angle) + unitData->r_horiz_offset*sin(t_angle + M_PI/2), angle_target, (plife > 0) ? plife : t->getRadius(), attack_type, t->getGroup(), t->get_id(), group->get_id());
			    }
			}
            // unit stuff
            set_dest(x, y);
			setAnimation(ANIMATION_DATA::RANGED, true);
		} else { // face target
			fight_timer--;
			rotate_towards((int)angle_target);
			if (!is_near_angle(angle_target, 20)) setAnimation(ANIMATION_DATA::STAND, true);
		}
	}

	// single units sync group position with their own
	if (group->is_single_unit()) group->sync_to(this);

	// update gemeric stuff
	update();
}

void AGENT::swap_positions(AGENT * ap)
{
	ap->update_swap();
	update_swap();

	int tmp = get_sort_id();

	sort_id = ap->get_sort_id();
	ap->set_sort_id(tmp);

	group->sort_units();
}

bool AGENT::is_last_swap(int sid)
{
	return (sid == last_swap);
}

void AGENT::draw(DISPLAY* display)
{
	int drawX = display->toScreenX((int) x);
	int drawY = display->toScreenY((int) y);

	// Draw
	BITMAP * frame;
	if (!build_data) frame = getFrame();
	else frame = build_data->sprite;

	if (highlight) circle(display->buffer, drawX, drawY, unitData->radius * 4 / (build_data ? 2 : 5), makecol(255, 255, 255)); // highlight
	pivot_sprite(display->buffer, frame, drawX, drawY, frame->w / 2, frame->h / 2, itofix( (int) (rotation / 360.0 * 256) ));
}

void AGENT::draw_selected(DISPLAY* display)
{
	int drawX = display->toScreenX((int) x);
	int drawY = display->toScreenY((int) y);
	circle(display->buffer, drawX, drawY, unitData->radius * 4 / (build_data ? 2 : 5), makecol(164, 24, 24)); // select circle
	arc(display->buffer, drawX, drawY, 0, ftofix((float) health / unitData->maxHealth * 255), unitData->radius * 4 / (build_data ? 2 : 5), makecol(28, 47, 226)); // health bar
}

void AGENT::check_health(void)
{
	if (health <= 0) die();
	if (health > unitData->maxHealth) health = unitData->maxHealth;
}

void AGENT::set_health(double health)
{
    this->health = health;
    if (this->health > unitData->maxHealth) this->health = unitData->maxHealth;
    else if (this->health <= 0) this->die();
}

void AGENT::add_health(double health)
{
    this->health += health;
    if (this->health > unitData->maxHealth) this->health = unitData->maxHealth;
    else if (this->health < 0) this->health = 0;
}

double AGENT::get_health(void)
{
    return health;
}

bool AGENT::isOnScreen(DISPLAY* display)
{
    BITMAP * frame = !build_data ? frame = getFrame() : build_data->sprite;

	if (this->x + frame->w > display->getWorldX() && this->x - frame->w < display->getWorldX() + display->getWidth() && this->y + frame->h > display->getWorldY() && this->y - frame->h < display->getWorldY() + display->getHeight()) return true;
	else return false;
}

void AGENT::drawToMiniMap(DISPLAY* display, int x, int y, double scale)
{
	int color = group->get_team_color();
	int toX = x + (int)(this->x * scale);
	int toY = y + (int)(this->y * scale);
	PUTPIXELS_SQUARE(display->buffer, toX, toY, color);

		// draw building build-point
	if (group->is_building() && group->is_sel()) group->draw_minmap_buildpoint(display, x, y, scale);
}

bool AGENT::is_fighting(void)
{
    return (state == AGENT::FIGHTING || state == AGENT::READY_FIGHT);
}

int AGENT::get_sort_id(void)
{
	return sort_id;
}

void AGENT::set_sort_id(int sid)
{
	sort_id = sid;
}

void AGENT::update_swap(void)
{
	last_swap = sort_id;
	swap_count = 40;
}

bool AGENT::start_rotate_to_group(void)
{
	rotate_to_group = true;
}

bool AGENT::end_rotate_to_group(void)
{
	rotate_to_group = false;
}

bool AGENT::at_unreachable_dest(void)
{
	return ( !group->is_moving() && !pather->passable((int)get_dest_x(), (int)get_dest_y()) );
}

void AGENT::die(void)
{
	health = 0;
	zombie = true;

	// create death particle
    if (unitData->death_particle[0]) part_sys.add(unitData->death_particle, x, y, rand()%360, -1, NULL, 0, 0, 0, false);
    if (unitData->blood_particle[0]) part_sys.add(unitData->blood_particle, x, y, rand()%360, -1, NULL, 0, 0, 0, false);
}

int AGENT::get_team(void)
{
	return group->get_team();
}

unsigned int AGENT::getGroup(void)
{
	return group_id;
}

char* AGENT::getType(void)
{
	return unitData->name;
}

bool AGENT::is_zombie(void)
{
	return zombie;
}

int AGENT::getRadius(void)
{
	return unitData->radius;
}

void AGENT::set_data(UNIT_DATA * data, BUILDING_DATA * bdata)
{
    unitData = data;
    build_data = bdata;
}

UNIT_DATA * AGENT::get_data(void)
{
    return unitData;
}

GROUP * AGENT::get_group_pointer(void)
{
	return group;
}

int AGENT::check_attackcollisions(int& xhit, int& yhit)
{	
	int r;

	GROUP* t = group->get_opponent();
	if (!t) return 0;

	list<GROUP*>* a = t->get_attackers();

	list<GROUP*>::iterator p;

	for (p = a->begin(); p != a->end(); p++) {
		r = (*p)->check_insidegroup_collision3(x, y, get_previous_x(), get_previous_y(), get_actual_speed(), get_id(), xhit, yhit);
		if (r) return r;
	}

	return 0;
}

bool AGENT::check_fight_collisions(void)
{
	int xhit, yhit, ttmp;
	if ((state == AGENT::READY_FIGHT || state == AGENT::FIGHTING) && !group->is_single_unit() && (ttmp = check_attackcollisions(xhit, yhit))) {
		
		double xup = (x > get_previous_x()) ? get_actual_speed() : -get_actual_speed();
		double yup = (y > get_previous_y()) ? get_actual_speed() : -get_actual_speed();
		
		switch (ttmp) {
			case 1:
			x = get_previous_x();
			y = get_previous_y();
			break;

			case 2:
			y = get_previous_y();
			x += xup;
			break;

			case 3:
			x = get_previous_x();
			y += yup;
			break;
			
			default:
			break;
		}

        double r = angle_to(xhit, yhit) * M_PI / 180;
		

		// rotate if not stuck
		if (!is_stuckcounter()) turn_fromto(angle_dest(), angle_to(group->get_x(), group->get_y()));
		if (has_not_moved()) stuckcounter++;
		else if (stuckcounter > 0) stuckcounter --;

		collision = PATHER::AGENT_COLLIDE;
		agent_collided = NULL;
		return false;
		return true;
    }
    return false;
}

void AGENT::hurt(double damage)
{
    if (!unitData->killable) return;
	health -= damage;
}

void AGENT::hurt_by(double damage, GROUP * gp)
{
    if (!unitData->killable) return;
	health -= damage;
	if (gp) {
        group->warn_attack(gp);
    }
}

void AGENT::hurt_by(double damage, AGENT * ap)
{
    if (!unitData->killable) return;
	health -= damage;
	if (ap) set_target(ap);
}

void AGENT::set_target(AGENT * ap)
{
	if (ap) {
		if (state == AGENT::READY_FIGHT) {
			target.switch_link(ap);
			state = AGENT::FIGHTING;
		}
		else if (state == AGENT::NORMAL) {
			group->warn_attack(ap->get_group_pointer());
		}
	}
}

AGENT::AGENT(GROUP * gp, UNIT_DATA * unitData, ANIMATION_DATA * animationData, BUILDING_DATA * bdp, PATHER * pp, PARTICLE_SYSTEM& part_sys, const ATTACK_TYPE * attack_type, int x, int y) : MOVE(unitData, pp, x, y), SPRITE(animationData), build_data(bdp), fight_timer(20), part_sys(part_sys), zombie(false), rotate_to_group(false), state(AGENT::NORMAL), attack_type(attack_type), last_swap(-1), swap_count(0), target(NULL), unitData(unitData), health(unitData->maxHealth), highlight(false), stuckcounter(0)
{
    agent = this;
    group = gp;
	this->group_id = group->get_id();
}

AGENT::~AGENT(void)
{
	unlink_all(); // unlink tasks
}

void AGENT::link(LINK<AGENT> * t)
{
	linked.push_back(t);
}

void AGENT::unlink(LINK<AGENT> * t)
{
	linked.remove(t);
}

void AGENT::unlink_all(void)
{
	list<LINK<AGENT>*>::iterator i;
	for (i = linked.begin(); i != linked.end(); i++) {
		(*i)->kill_link();
	}
	linked.clear();
}

bool AGENT::is_team_collision(void)
{
    return (collision == PATHER::AGENT_COLLIDE && agent_collided && agent_collided->get_team() == get_team());
}
