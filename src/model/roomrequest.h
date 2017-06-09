#ifndef ROOMREQUEST_H
#define ROOMREQUEST_H

#include "src/server/ormlite.h"


struct RoomRequest
{
    // 用户名
    int room_id;

    // 开关机次数
    int begin_time;
    int stop_time;
    int begin_temp;
    int stop_temp;
    int begin_speed;
    int stop_speed;
    double money;

    ORMAP(RoomRequest, room_id, begin_time, stop_time, begin_temp, stop_temp, begin_speed, stop_speed, money);

};

#endif // ROOMREQUEST_H
