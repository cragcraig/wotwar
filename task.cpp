#include "wotwar.h"

// TASK class functions

// blank task
TASK::TASK(void) : type(TASK::NONE), pNext(NULL), specific_group(0), attacklisted(NULL)
{

}

// most tasks
TASK::TASK(TASK::TASK_TYPE t, int player_id, int a, int b, bool make_current, bool is_final_dest, int specific_group) : type(t), link(NULL), linked_flag(false), specific_group(specific_group), makeCurrent(make_current), is_final(is_final_dest), groupUnits(0), pathfind_level(0), num_assigned(0), is_pathfind(false), replace_next(false), player(player_id), message_only(false), checked_flag(false), sig_flag(false), generic_counter(0), done_flag(false), startCount(false), gettingInFormationFlag(false), unit_data(NULL), pNext(NULL), attacklisted(NULL)
{
	id = newId();
	pId = newPathId();

	// init network message
	net_msg.type = MSGTYPE_TASK;
	net_msg.msg.task.type = t;
	net_msg.msg.task.pid = player_id;
	net_msg.msg.task.a = a;
	net_msg.msg.task.b = b;
	net_msg.msg.task.make_current = make_current;
	net_msg.msg.task.is_final_dest = is_final_dest;

	// setup task
	switch (t) {
		case TASK::MOVE :
			this->x = this->groupX = a;
			this->y = this->groupY = b;
		break;

		case TASK::ROTATE :
			this->theta = a;
		break;

		case TASK::WAIT :
			this->count = a;
		break;

		case TASK::WAIT_SIG :
		break;

		case TASK::WAIT_COLLISION :
		break;

		case TASK::SIGNAL :
			this->message_only = true;
			this->id = a;
		break;

		case TASK::CHECK_WAIT :
			this->message_only = true;
			this->sig_flag = true;
			this->id = a;
		break;

		case TASK::BUILD :
		case TASK::BUILD_BUILDING :
		case TASK::CREATE_BUILDING :
            this->build_id = a; // type
            this->counter = this->buildtime = b;
            this->generic_counter = 0;
            this->startCount = false;
		break;

		case TASK::UNBUILD :
            this->build_id = a; // type
		break;
	}
}

// fight task
TASK::TASK(TASK::TASK_TYPE t, int player_id, GROUP * gp, bool make_current, int specific_group) : type(t), link(gp), linked_flag(true), specific_group(specific_group), makeCurrent(make_current), groupUnits(0), pathfind_level(0), num_assigned(0), is_pathfind(false), replace_next(false), player(player_id), message_only(false), checked_flag(false), sig_flag(false), done_flag(false), startCount(false), gettingInFormationFlag(false), unit_data(NULL), pNext(NULL), is_final(false), attacklisted(NULL)
{
	id = newId();
	pId = newPathId();

	// init network message
	net_msg.type = MSGTYPE_FIGHT;
	net_msg.msg.fight.type = t;
	net_msg.msg.fight.pid = player_id;
	net_msg.msg.fight.enemy_pid = gp->get_team();
	net_msg.msg.fight.enemy_id = gp->get_group_id();
	net_msg.msg.fight.make_current = make_current;

	// so drawing paths works
	x = (int)gp->get_x();
	y = (int)gp->get_y();

	// link task to agent
	relink();
}

// network sync (dummy) task
TASK::TASK(MESSAGE * message) : type(TASK::GRPSYNC), link(NULL), linked_flag(false), player(0), specific_group(0), message_only(true), unit_data(NULL), pNext(NULL), attacklisted(NULL)
{
    id = newId();
	pId = newPathId();

	// copy network message
	net_msg = *message;

	// set up grpsync message routing
	if (net_msg.type == MSGTYPE_GRPSYNC) {
        specific_group = net_msg.msg.gsync.group_id;
        player = net_msg.player_id;
	}
}

// destructor
TASK::~TASK(void)
{
	if (link && attacklisted) link->remove_attacker(attacklisted);
	if (link) link->unlink_task(this);
}

// unlink
void TASK::unlink(void)
{
	link = NULL;
}

// relink
void TASK::relink(void)
{
	if (link) link->link_task(this);
}

// fix tasks that point off map
void TASK::fit_to_map(WORLD * w)
{
    if (x - 100 < 0) x = groupX = 100;
    if (y - 100 < 0) y = groupY = 100;
    if (x + 100 > w->getWidth()) x = groupX = w->getWidth() - 100;
    if (y + 100 > w->getHeight()) y = groupY = w->getHeight() - 100;
}

// task id generator
unsigned int TASK::newId(void)
{
	static unsigned int id_count = 0;
	return id_count += 10;
}

// path id generator
unsigned int TASK::newPathId(void)
{
	static unsigned int id_count = 0;
	return id_count++;
}
