#include "message.h"

#define NETMSG_COPY(type, offset, array, value) {*(((type)*)((array)+(offset))) = (value);}

char* netmsg_to_none(MESSAGE* msg)
{
    new uint8_t bytes[4];
    NETMSG_COPY(uint16_t, 0, bytes, msg->size);
    NETMSG_COPY(uint8_t, 2, bytes, msg->type);
    NETMSG_COPY(uint8_t, 3, bytes, msg->player_id);

    return bytes;
}

char* netmsg_to_task(MESSAGE* msg)
{
    new uint8_t bytes[4+sizeof(struct MESSAGE_TASK)];
    NETMSG_COPY(uint16_t, 0, bytes, msg->size);
    NETMSG_COPY(uint8_t, 2, bytes, msg->type);
    NETMSG_COPY(uint8_t, 3, bytes, msg->player_id);

    NETMSG_COPY(uint32_t, 4, bytes, msg->msg.task.group_id);
    NETMSG_COPY(uint32_t, 8, bytes, msg->msg.task.type);
    NETMSG_COPY(uint8_t, 12, bytes, msg->msg.task.pid);

    NETMSG_COPY(uint32_t, 13, bytes, msg->msg.task.a);
    NETMSG_COPY(uint32_t, 17, bytes, msg->msg.task.b);
    NETMSG_COPY(uint8_t, 21, bytes, msg->msg.task.make_current);
    NETMSG_COPY(uint8_t, 22, bytes, msg->msg.task.is_final_dest);

    return bytes;
}

char* netmsg_to_fight(MESSAGE* msg)
{
    new uint8_t bytes[4+sizeof(struct MESSAGE_FIGHT)];
    NETMSG_COPY(uint16_t, 0, bytes, msg->size);
    NETMSG_COPY(uint8_t, 2, bytes, msg->type);
    NETMSG_COPY(uint8_t, 3, bytes, msg->player_id);

    NETMSG_COPY(uint32_t, 4, bytes, msg->msg.fight.group_id);
    NETMSG_COPY(uint32_t, 8, bytes, msg->msg.fight.type);
    NETMSG_COPY(uint8_t, 12, bytes, msg->msg.fight.pid);

    NETMSG_COPY(uint8_t, 13, bytes, msg->msg.fight.enemy_pid);
    NETMSG_COPY(uint32_t, 14, bytes, msg->msg.fight.enemy_id);
    NETMSG_COPY(uint8_t, 18, bytes, msg->msg.fight.make_current);

    return bytes;
}

char* netmsg_to_chat(MESSAGE* msg)
{
    new uint8_t bytes[6+msg->msg.chat.len];
    NETMSG_COPY(uint16_t, 0, bytes, msg->size);
    NETMSG_COPY(uint8_t, 2, bytes, msg->type);
    NETMSG_COPY(uint8_t, 3, bytes, msg->player_id);

    NETMSG_COPY(uint16_t, 4, bytes, msg->msg.chat.len);
    for (int i=0; i<msg->msg.chat.len; i++)
        NETMSG_COPY(uint8_t, 6+i, bytes, msg->msg.chat.str[i]);
}

char* netmsg_to_grpsync(MESSAGE* msg)
{
    new uint8_t bytes[4+sizeof(struct MESSAGE_GRPSYNC)];
    NETMSG_COPY(uint16_t, 0, bytes, msg->size);
    NETMSG_COPY(uint8_t, 2, bytes, msg->type);
    NETMSG_COPY(uint8_t, 3, bytes, msg->player_id);

    NETMSG_COPY(uint32_t, 4, bytes, msg->msg.gsync.group_id);
    NETMSG_COPY(uint32_t, 8, bytes, msg->msg.gsync.type);
    NETMSG_COPY(uint8_t, 12, bytes, msg->msg.gsync.pid);
    NETMSG_COPY(uint32_t, 13, bytes, msg->msg.gsync.grpsync_type);
    NETMSG_COPY(uint32_t, 17, bytes, msg->msg.gsync.target_id);
    NETMSG_COPY(uint32_t, 21, bytes, msg->msg.gsync.x);
    NETMSG_COPY(uint32_t, 25, bytes, msg->msg.gsync.y);
    NETMSG_COPY(uint8_t, 29, bytes, msg->msg.gsync.count);
    for (int i=0; i<msg->msg.gsync.count; i++)
        NETMSG_COPY(uint16_t, 30+2*i, bytes, msg->msg.gsync.data[i]);

}

char* netmsg_to_array(MESSAGE* msg)
{
    switch (msg->type) {
        case MSGTYPE_NONE: return netmsg_to_none(msg);
        case MSGTYPE_TASK: return netmsg_to_task(msg);
        case MSGTYPE_FIGHT: return netmsg_to_fight(msg);
        case MSGTYPE_CHAT: return netmsg_to_chat(msg);
        case MSGTYPE_GRPSYNC: return netmsg_to_grpsync(msg);
    }
}



void netmsg_from_array(char* array, MESSAGE* msg)
{

}
