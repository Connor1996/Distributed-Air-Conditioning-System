#include "dispatch.h"
#include "ormlite.h"

#include "../protocol.h"
#include "../model/userinfo.h"

#include <stdexcept>
#include <algorithm>
#include <unordered_set>

using namespace ORMLite;


std::string Dispatcher::Dispatch(json requestInfo)
{
    json responseInfo;
    // 根据请求信息内容，转到相应的处理逻辑
    switch (requestInfo["op"].get<int>()) {
    case LOG_IN_USER:
        responseInfo = LoginHandle(requestInfo);
        break;
    case REQ_STOP:
        _state.isOn = false;
        break;
    case REQ_RESUME:
        _state.isOn = true;
        break;
    case REQ_UPDATE:
        responseInfo = UpdateSettingHandle(requestInfo);
        break;
    case REPORT_STATE:
        responseInfo = StateHandle(requestInfo);
        break;
    default:
        std::cout << "[ERROR] Bad request" << std::endl;
        break;
    }

    return std::move(responseInfo.dump());
}

json Dispatcher::LoginHandle(json &requestInfo)
{
    ORMapper<UserInfo> mapper("data.db");
    UserInfo helper;
    QueryMessager<UserInfo> messager(helper);

    json responseInfo;

    std::cout << "[INFO] Login request comes" << std::endl;

    // 查询数据库是否有该用户名同时为该密码的条目
    auto result = mapper.Query(messager
                 .Where(Field(helper.room_id) == requestInfo["room_id"].get<std::string>()
                        && Field(helper.user_id) == requestInfo["user_id"].get<std::string>()));

    if (result)
    {
        // 如果没有说明，说明用户名或密码错误
        if (messager.IsNone())
            responseInfo["ret"] = LOG_IN_FAIL;
        else
        {
            // 将username加入在线列表
            _username = requestInfo["user_id"].get<std::string>();
            // 检查是否已经在线
            if (_parent->Online(_username, this))
                responseInfo["ret"] = LOG_IN_SUCC;
            else
                responseInfo["ret"] = LOG_IN_FAIL;
        }
    }
    else
    {
        responseInfo["define"] = LOG_IN_FAIL;
        std::cout << "[ERROR] " << mapper.GetErrorMessage() << std::endl;
    }

    return responseInfo;
}

json Dispatcher::UpdateSettingHandle(json &requestInfo)
{
    // 从控机更改设置：工作模式、设定温度、风速大小
    // int op = REQ_UPDATE
    // bool is_heat_mode
    // int temp
    // int speed = 1..3

    _state.isHeatMode = requestInfo["is_heat_mode"].get<bool>();
    _state.temperature = requestInfo["temp"].get<int>();
    _state.speed = requestInfo["speed"].get<int>();

    json responseInfo;
//    = {
//            {"ret", REPLY_CON},
//            {"is_valid", _state.isHeatMode == _parent->_setting.isHeatMode}
//        };
    return responseInfo;
}

json Dispatcher::StateHandle(json &requestInfo)
{
    json responseInfo;
    return responseInfo;
}

void Dispatcher::Logout()
{
    _parent->Offline(_username);
}
