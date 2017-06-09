#ifndef ROOMINFO_H
#define ROOMINFO_H


#include "../server/ormlite.h"


struct RoomInfo
{
    // 用户名
    int room_id;

    // 开关机次数
    int count;

    ORMAP(RoomInfo, room_id, count);

};


#endif // ROOMINFO_H
