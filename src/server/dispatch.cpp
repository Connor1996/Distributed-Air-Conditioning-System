#include "dispatch.h"
#include "ormlite.h"

#include "../protocol.h"
#include "../model/userinfo.h"

#include <stdexcept>
#include <algorithm>
#include <unordered_set>

using namespace ORMLite;
using std::cout;
#include <QMessageBox>
std::string Dispatcher::Dispatch(json requestInfo)
{
    json responseInfo;
    // 根据请求信息内容，转到相应的处理逻辑
    switch (requestInfo["op"].get<int>()) {
    case LOG_IN_USER:
        responseInfo = LoginHandle(requestInfo);
        break;
    case REQ_STOP:
        cout << "stop";
        _state.isOn = false;
        break;
    case REQ_RESUME:
        cout << "resume";
        _state.isOn = true;
        break;
    case REQ_UPDATE:
        cout << "update";
        responseInfo = UpdateSettingHandle(requestInfo);
        break;
    case REPORT_STATE:
        cout << "state";
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
    json responseInfo;

    std::cout << "[INFO] Login request comes" << std::endl;
    auto roomId = requestInfo["room_id"].get<int>();
    auto userId = requestInfo["user_id"].get<std::string>();

    // 查询数据库是否有该用户名同时为该密码的条目
    responseInfo["ret"] = LOG_IN_FAIL;
    if (_parent->_rooms.find(roomId) != _parent->_rooms.end()) {
        if (_parent->_rooms[roomId] == userId) {
            // 检查是否已经在线
            if (_parent->Online(roomId, this)) {
                responseInfo["ret"] = LOG_IN_SUCC;
                responseInfo["is_heat_mode"] = true;
                responseInfo["temp"] = 0;
            }
        }
    }


    return responseInfo;
}

json Dispatcher::UpdateSettingHandle(json &requestInfo)
{
    _state.isHeatMode = requestInfo["is_heat_mode"].get<bool>();
    _state.setTemperature = requestInfo["temp"].get<int>();
    _state.speed = requestInfo["speed"].get<int>();

    json responseInfo = {
        {"ret", REPLY_CON},
        {"is_valid", _state.isHeatMode == _parent->GetSetting().isHeatMode}
    };

    return responseInfo;
}

json Dispatcher::StateHandle(json &requestInfo)
{
    _state.realTemperature = requestInfo["real_temp"].get<int>();
    json responseInfo = {
        {"ret", REPLY_MONEY},
        {"money", 0},
        {"power", 0}
    };

    return responseInfo;
}

void Dispatcher::Logout()
{
    _parent->Offline(_username);
}
