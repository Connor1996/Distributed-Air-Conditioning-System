#ifndef USERINFO_H
#define USERINFO_H

#ifdef __SERVER__
#include "../server/ormlite.h"

#endif


struct UserInfo
{
    // 用户名
    std::string room_id;
    // 密码
    std::string user_id;


#ifdef __SERVER__
    ORMAP(UserInfo, room_id, user_id);
#endif

};

#endif // USERINFO_H
