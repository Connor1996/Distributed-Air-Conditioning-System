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
                _roomId = roomId;
            }
        }
    }


    return responseInfo;
}

json Dispatcher::StateHandle(json &requestInfo)
{
    _state.isHeatMode = requestInfo["is_heat_mode"].get<bool>();
    _state.realTemperature = requestInfo["real_temp"].get<int>();
    _state.setTemperature = requestInfo["set_temp"].get<int>();
    _state.speed = requestInfo["speed"].get<int>();

    bool isValid = false;
    if (_parent->_setting.isPowerOn && requestInfo["is_on"].get<bool>() &&
            _state.isHeatMode == _parent->_setting.isHeatMode) {
        if (_state.isHeatMode && _state.setTemperature > _state.realTemperature)
            isValid = true;
        else if (!_state.isHeatMode && _state.setTemperature < _state.realTemperature)
            isValid = true;
    }

    _state.isOn = isValid;
    json responseInfo = {
        {"ret", REPLY_CON},
        {"is_valid", isValid},
        {"money", 0},
        {"power", 0},
    };

    return responseInfo;
}

void Dispatcher::Logout()
{
    _parent->Offline(_roomId);
}
