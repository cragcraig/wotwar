#ifndef MOVE_H
#define MOVE_H

#include "wotwar_classes.h"
#include "pather.h"

class UNIT_DATA;
class GROUP;
class COLLISION;
class AGENT;

class MOVE
{
	private:
		double speed, actual_speed, rotate_rate, dest_x, dest_y, speed_rate;
		UNIT_DATA * unit_data_move;
		double old_x, old_y;

	protected:
		double x, y, rotation;
		COLLISION * collide_node;
		bool update_collision;
		PATHER::COLLISION_TYPE collision;
		AGENT* agent_collided;
		unsigned int id;
		PATHER * pather;
		GROUP * group;
		AGENT * agent;

		void collision_check(void);
		void collision_avoid(void);
		static unsigned int newId(void); // gives a unique ID
		void rotate_towards(int direction);
		void move(int goX, int goY, bool enable_close_movement=false);
		void shift(double dist, double angle);
		bool within_dist(int toX, int toY, int distance);
		bool within_dist_dest(int distance);
		bool is_at(double toX, double toY);
		bool is_exact_dest(void);
		bool is_angle(double theta);
		int rotation_to_dest(void);
		void hold_pos(void);
		void move_stop(void);
		void move_mult(double s);
		void set_speed(double speed, double speed_rate=1.0);
		void rotate(double theta);
		void compute_move(int dist, int * x1, int * y1, double rot=-9999);

	public:
		MOVE(UNIT_DATA * udp, PATHER * pp, double x, double y);
		virtual ~MOVE(void);
		void update_move(void);
		double get_previous_x(void);
		double get_previous_y(void);
		double get_x(void);
		double get_y(void);
		double get_speed(void);
		double get_actual_speed(void);
		double get_xspeed(void);
		double get_yspeed(void);
		double get_rotation(void);
		double get_rotation_rad(void);
		double get_angle_dest(void);
		double get_dest_x(void);
		double get_dest_y(void);
		unsigned int get_id(void) {return id;}
		void set_id(int i) {id = i;}
		bool is_moving(void);
		bool is_stopped(void);
		bool is_near(double toX, double toY);
		bool is_dest(void);
		bool is_near_dest(void);
		bool is_dest_angle(void);
		bool is_near_dest_angle(double off=15);
		bool is_dest_angle_near(double theta, double off=15);
		bool in_box(int x1, int y1, int x2, int y2);
		double dist_to(double toX, double toY);
		double dist_dest(void);
		double angle_dest(void);
		double angle_to(double toX, double toY);
		bool has_not_moved(void);
		void turn_away(double theta);
		void turn_fromto(double theta_from, double theta_to);
		bool is_near_angle(double theta, double off=15);
		void set_loc(double new_x, double new_y);
		void set_rotation(double rot);
		void set_dest(double x, double y);
		void shorten_dest(double dist);
		void set_collide_node(AGENT * ap);
		COLLISION * get_collide_node(void);
		void sync_to(MOVE * g);
};

#endif
