#include "dispatch.h"
#include "ormlite.h"

#include "../protocol.h"
#include "../model/userinfo.h"

#include <stdexcept>
#include <algorithm>
#include <unordered_set>

using namespace ORMLite;

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
        QMessageBox::information(this, "info", "stop");
        _state.isOn = false;
        break;
    case REQ_RESUME:
        QMessageBox::information(this, "info", "resume");
        _state.isOn = true;
        break;
    case REQ_UPDATE:
        QMessageBox::information(this, "info", "update");
        responseInfo = UpdateSettingHandle(requestInfo);
        break;
    case REPORT_STATE:
        QMessageBox::information(this, "info", "state");
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
    auto roomId = requestInfo["room_id"].get<std::string>();
    auto userId = requestInfo["user_id"].get<std::string>();

    std::cout << "[INFO] Login request comes" << std::endl;

    // 查询数据库是否有该用户名同时为该密码的条目
    responseInfo["ret"] = LOG_IN_FAIL;
    if (_rooms.find(roomId) != _room.end()) {
        if (_rooms[roomId] == userId) {
            // 检查是否已经在线
            if (_parent->Online(_roomId, this))
                responseInfo["ret"] = LOG_IN_SUCC;
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
