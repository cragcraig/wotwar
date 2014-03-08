#ifndef MESSAGE_H
#define MESSAGE_H

#include <boost/cstdint.hpp>

// functions
//char* netmsg_to_array(MESSAGE* msg);
//void netmsg_from_array(char* array, MESSAGE* msg);

// message types

// disable padding!
#pragma pack(push)
#pragma pack(1)

enum MESSAGE_TYPE {MSGTYPE_NONE=0, MSGTYPE_TASK, MSGTYPE_FIGHT, MSGTYPE_CHAT, MSGTYPE_GRPSYNC};

struct MESSAGE_TASK
{
    uint32_t group_id;
    uint32_t type;
    uint8_t pid;
    int32_t a;
    int32_t b;
    uint8_t make_current;
    uint8_t is_final_dest;
};

struct MESSAGE_FIGHT
{
    uint32_t group_id;
    uint32_t type;
    uint8_t pid;
    uint8_t enemy_pid;
    int32_t enemy_id;
    uint8_t make_current;
};

enum GRPSYNC_TYPE {GRPSYNC_GROUP=0, GRPSYNC_BUILDING, GRPSYNC_TARGET};

struct MESSAGE_GRPSYNC
{
    uint32_t group_id;
    uint32_t type;
    uint8_t pid;
    uint32_t grpsync_type;
    uint32_t target_id;
    uint32_t x;
    uint32_t y;
    uint8_t count;
    uint16_t data[GROUP_MAXUNITS];
};

struct MESSAGE_CHAT
{
    uint16_t len;
    char * str; // only used locally
};

// message containers

union MESSAGE_BODY
{
    MESSAGE_TASK task;
    MESSAGE_FIGHT fight;
    MESSAGE_GRPSYNC gsync;
    MESSAGE_CHAT chat;
};

struct MESSAGE
{
    uint16_t size;      // message size
    uint8_t type;       // type of message
    uint8_t player_id;  // id of player initiating message
    MESSAGE_BODY msg;   // actual data structure
};

#pragma pack(pop)

#endif
