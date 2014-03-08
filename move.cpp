#include "wotwar.h"

MOVE::MOVE(UNIT_DATA * udp, PATHER * pp, double x, double y) : unit_data_move(udp), x(x), y(y), old_x(x), old_y(y), agent(NULL), group(NULL), rotation(90), actual_speed(0), rotate_rate(0), speed(0), dest_x(x), dest_y(y), speed_rate(1), pather(pp), collide_node(NULL), collision(PATHER::NO_COLLIDE)
{
	id = newId();
}

MOVE::~MOVE(void)
{
	// Delete collide_node and clear references
	if (collide_node) {
        //if (pather->is_on_map(get_previous_x(), get_previous_y())) pather->clear((int)get_previous_x(), (int)get_previous_y(), collide_node);
        if (collide_node->x >= 0 && collide_node->y >= 0)
            pather->clear((int)collide_node->x, (int)collide_node->y, collide_node);
        delete collide_node;
	}
}

void MOVE::rotate_towards(int direction)
{
	if (direction - rotation < -180) direction += 360;
	if (direction - rotation > 180) direction -= 360;

	int difference = (int) (direction - rotation + 0.5);

	rotate_rate = abs(difference) / 180.0 * (unit_data_move->quickRotateRate - unit_data_move->baseRotateRate) + unit_data_move->baseRotateRate;

	// Do the rotating
	if (difference < -rotate_rate) rotation -= rotate_rate;
	else if (difference > rotate_rate) rotation += rotate_rate;
	else rotation = direction;

	// Wrap the angle around
	if (rotation > 360) rotation -= 360;
	else if (rotation < 0) rotation += 360;
}

void MOVE::move(int goX, int goY, bool enable_close_movement)
{
	// don't bother if at destination
	if (is_dest()) return;

	// Find angle
	int direction = (int)angle_to(goX, goY);

	// Make sure the turn is less than 180 degrees
	if (direction - rotation < -180) direction += 360;
	if (direction - rotation > 180) direction -= 360;

	int difference = (int) (direction - rotation + 0.5);
	double distance = dist_to(goX, goY);

	if (difference != 0 || speed != unit_data_move->maxSpeed) {
		if (enable_close_movement && (distance / unit_data_move->maxSpeed) * unit_data_move->baseRotateRate < abs(2 * difference) && distance < unit_data_move->maxTurnRadius) { // only do if next task is not a movement task if it getting stuck going in circles on tight corners, but that shouldn't matter once pathfinding is added as the nodes should be far enough apart. The code is "!taskList.nextIsMove() &&".
			// If within the max turn diameter from target area then rotate and calculate speed needed to hit target dead on (ignoring acceleration).
			rotate_rate = unit_data_move->avgRotateRate;
			speed = dist_to(goX, goY) / 2.0 * sin(rotate_rate * M_PI / 180.0) / sin((180.0 - rotate_rate) / 2.0 * M_PI / 180.0);
		}
		else {
			// Slow down and turn more quickly when the difference in directions is very large
			rotate_rate = abs(difference) / 180.0 * (unit_data_move->quickRotateRate - unit_data_move->baseRotateRate) + unit_data_move->baseRotateRate;
			speed = (1.0 - abs(difference) / 180.0) * (unit_data_move->maxSpeed - unit_data_move->minSpeed) + unit_data_move->minSpeed;
		}
	}

	// Do the rotating
	if (difference < -rotate_rate * speed_rate) rotation -= rotate_rate * speed_rate;
	else if (difference > rotate_rate * speed_rate) rotation += rotate_rate * speed_rate;
	else rotation = direction;

	// Wrap the angle around
	if (rotation > 360) rotation -= 360;
	else if (rotation < 0) rotation += 360;
}

void MOVE::update_move(void)
{
	// Accelerate / Deccelerate
	if (speed > unit_data_move->maxSpeed) speed = unit_data_move->maxSpeed;
	if (actual_speed < 0) actual_speed = 0;
	else if (!speed && actual_speed < unit_data_move->acceleration * 7) actual_speed = 0; // If too slow, just stop.
	else if (actual_speed < speed) actual_speed += unit_data_move->acceleration;
	else if (actual_speed > speed) actual_speed -= unit_data_move->acceleration;

    // store old values
    old_x = x;
	old_y = y;

	// Compute move
	x = x + cos(rotation / 180.0 * M_PI) * actual_speed * speed_rate;
	y = y + sin(rotation / 180.0 * M_PI) * actual_speed * speed_rate;
}

void MOVE::collision_check(void)
{
	bool acol = false;
    double xprev = get_previous_x();
    double yprev = get_previous_y();
    // Perfect collision checking while fighting
    if (agent) acol = agent->check_fight_collisions();
	// Collision checking and actual movement
	update_collision = false;
	if (!get_actual_speed()) { // haven't moved
		if (!acol) collision = PATHER::NO_COLLIDE;
		return;
	} else if ((int)x/pather->patchSize == (int)xprev/pather->patchSize && (int)y/pather->patchSize == (int)yprev/pather->patchSize) { // haven't moved enough
		if (!acol) collision = PATHER::NO_COLLIDE;
		return;
	}
	// have moved enough, need to update collision squares
	update_collision = true;
	if (pather->passable_unit((int)x, (int)y, collide_node)) {
		collision = PATHER::NO_COLLIDE;
	} else {
		collision = pather->collision_type((int)x, (int)y);
		if (pather->passable_unit((int)x, (int)yprev, collide_node)) y = yprev;
		else if (pather->passable_unit((int)xprev, (int)y, collide_node)) x = xprev;
	}
	if (collision == PATHER::AGENT_COLLIDE) agent_collided = pather->who_collision((int)x, (int)y);
	else agent_collided = NULL;
	pather->set_autounset((int)x, (int)y, collide_node);
}

void MOVE::collision_avoid(void)
{
	int dir;
	int rot = (int)angle_to(get_dest_x(), get_dest_y());

	// different style depending on collision type
	if (group && collision == PATHER::STATIONARY_COLLIDE) dir = (int)angle_to((int)group->get_x(), (int)group->get_y());
	else dir = angle_dest(); //(int)group->get_rotation();

	// Make sure the turn is less than 180 degrees
	if (dir - rot < -180) dir += 360;
	if (dir - rot > 180) dir -= 360;

	int difference = (int) (dir - rot + 0.5);

	// do the rotating
	double rrate = (unit_data_move->quickRotateRate + unit_data_move->baseRotateRate)/2 + unit_data_move->baseRotateRate;
	if (difference > 0) rotate(rrate);
	else rotate(-rrate);

	// set move speed
	set_speed(unit_data_move->maxSpeed, 1.0);
}

void MOVE::shift(double dist, double angle)
{
	x += cos(angle / 180.0 * M_PI) * dist;
	y += sin(angle / 180.0 * M_PI) * dist;
}

double MOVE::angle_to(double toX, double toY)
{
	return atan2(toY - y, toX - x) / M_PI * 180.0;
}

bool MOVE::within_dist(int toX, int toY, int distance)
{
	if ((x - toX) * (x - toX) + (y - toY) * (y - toY) <= distance * distance) return true;
	else return false;
}

bool MOVE::within_dist_dest(int distance)
{
	if ((x - dest_x) * (x - dest_x) + (y - dest_y) * (y - dest_y) <= distance * distance) return true;
	else return false;
}

double MOVE::dist_dest(void)
{
	return dist_to(dest_x, dest_y);
}

bool MOVE::is_at(double toX, double toY)
{
	int distance = (int) dist_to((int) toX, (int) toY);
	if (distance < unit_data_move->radius || distance <= (actual_speed * actual_speed) / (2 * unit_data_move->acceleration)) return true;
	else return false;
}

bool MOVE::is_near(double toX, double toY)
{
	if (dist_to((int) toX, (int) toY) < unit_data_move->radius * 2.5) return true;
	else return false;
}

double MOVE::dist_to(double toX, double toY)
{
	return sqrt((x - toX) * (x - toX) + (y - toY) * (y - toY));
}

void MOVE::set_collide_node(AGENT * ap)
{
	if (collide_node) return;
	collide_node = new COLLISION(ap);
	agent = ap;
}

COLLISION * MOVE::get_collide_node(void)
{
    return collide_node;
}

void MOVE::set_dest(double x, double y)
{
	dest_x = x;
	dest_y = y;
}

void MOVE::shorten_dest(double dist)
{
    dest_x += dist * cos(get_angle_dest() * M_PI / 180. + M_PI);
    dest_y += dist * sin(get_angle_dest() * M_PI / 180. + M_PI);
}

bool MOVE::is_dest(void)
{
	return is_at(dest_x, dest_y);
}

bool MOVE::is_near_dest(void)
{
	return is_near(dest_x, dest_y);
}

bool MOVE::is_exact_dest(void)
{
	return (abs((int)(dest_x - x)) + abs((int)(dest_y - y)) < 4);
}

bool MOVE::is_dest_angle(void)
{
	return is_angle(angle_to(dest_x, dest_y));
}

bool MOVE::is_near_dest_angle(double off)
{
	return is_near_angle(angle_to(dest_x, dest_y), off);
}

double MOVE::get_x(void)
{
	return x;
}

double MOVE::get_y(void)
{
	return y;
}

double MOVE::get_rotation(void)
{
	return rotation;
}

double MOVE::get_rotation_rad(void)
{
	return rotation * M_PI / 180.;
}

double MOVE::get_previous_x(void)
{
    return old_x;
}

double MOVE::get_previous_y(void)
{
    return old_y;
}

void MOVE::compute_move(int dist, int * x1, int * y1, double rot)
{
    if (rot < -800) rot = rotation;
    *x1 = (int)(x + dist * cos(rot * M_PI / 180));
    *y1 = (int)(y + dist * sin(rot * M_PI / 180));
}

int MOVE::rotation_to_dest(void)
{
	double theta = get_angle_dest();
	if (theta - rotation < -180) theta += 360;
	if (theta - rotation > 180) theta -= 360;
	return (int) (theta - rotation + 0.5);
}

void MOVE::turn_away(double theta)
{
	if (theta - rotation < -180) theta += 360;
	if (theta - rotation > 180) theta -= 360;
	if (theta - rotation + 0.5 > 0) rotation -= 2*unit_data_move->quickRotateRate;
	else if (theta - rotation + 0.5 < 0) rotation += 2*unit_data_move->quickRotateRate;
}

void MOVE::turn_fromto(double theta_from, double theta_to)
{
	if (theta_from - theta_to < -180) theta_from += 360;
	if (theta_from - theta_to > 180) theta_from -= 360;
	if (theta_from - theta_to + 0.5 > 0) rotation += 2*unit_data_move->quickRotateRate;
	else if (theta_from - theta_to + 0.5 < 0) rotation -= 2*unit_data_move->quickRotateRate;
}

unsigned int MOVE::newId(void)
{
	static unsigned int id_count = 0;
	return id_count++;
}

void MOVE::sync_to(MOVE * g)
{
	x = g->get_x();
	y = g->get_y();
	dest_x = g->get_dest_x();
	dest_y = g->get_dest_y();
	rotation = g->get_rotation();
	actual_speed = g->get_actual_speed();
	speed = g->get_speed();
}

void MOVE::set_speed(double speed, double speed_rate)
{
    this->speed = speed;
    this->speed_rate = speed_rate;
}

void MOVE::rotate(double theta)
{
    rotation += theta;
    if (rotation > 360) rotation -= 360;
	else if (rotation < 0) rotation += 360;
}

double MOVE::angle_dest(void)
{
    return angle_to(dest_x, dest_y);
}

double MOVE::get_dest_x(void)
{
	return dest_x;
}

double MOVE::get_dest_y(void)
{
	return dest_y;
}

double MOVE::get_actual_speed(void)
{
	return actual_speed;
}

double MOVE::get_angle_dest(void)
{
	return angle_to(dest_x, dest_y);
}

void MOVE::move_stop(void)
{
	speed = 0.;
}

void MOVE::hold_pos(void)
{
    speed = 0.;
    actual_speed = 0.;
}

double MOVE::get_speed(void)
{
    return speed;
}

double MOVE::get_xspeed(void)
{
    return speed * cos(rotation * M_PI/180.);
}

double MOVE::get_yspeed(void)
{
    return speed * sin(rotation * M_PI/180.);
}

bool MOVE::is_angle(double theta)
{
	return is_near_angle(theta, rotate_rate);
}

bool MOVE::is_near_angle(double theta, double off)
{
    if (theta - rotation < -180) theta += 360;
	if (theta - rotation > 180) theta -= 360;
	return (abs((int)(theta - rotation)) <= off);
}

bool MOVE::is_dest_angle_near(double theta, double off)
{
    int dest_angle = get_angle_dest();
    if (theta - dest_angle < -180) theta += 360;
	if (theta - dest_angle > 180) theta -= 360;
	return (abs((int)(theta - dest_angle)) <= off);
}

bool MOVE::is_moving(void)
{
	return (actual_speed >= unit_data_move->minSpeed);
}

bool MOVE::is_stopped(void)
{
    return (actual_speed == 0.);
}

bool MOVE::has_not_moved(void)
{
	return ((int)x == (int)get_previous_x() && (int)y == (int)get_previous_y());
}

void MOVE::set_loc(double new_x, double new_y)
{
	x = new_x;
	y = new_y;
}

void MOVE::set_rotation(double rot)
{
	rotation = rot;
}

bool MOVE::in_box(int x1, int y1, int x2, int y2)
{
	// Make box corners sane
	int tmp;
	if (x1 > x2) {
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if (y1 > y2) {
		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	if (this->x >= x1 && this->x <= x2 && this->y >= y1 && this->y <= y2) return true;
	else return false;
}

void MOVE::move_mult(double s)
{
	speed_rate = s;
}
