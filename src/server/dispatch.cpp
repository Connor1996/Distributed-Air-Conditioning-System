#include "dispatch.h"
#include "ormlite.h"

#include "../protocol.h"
#include "../model/userinfo.h"

#include <stdexcept>
#include <algorithm>
#include <unordered_set>
#include <ctime>

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
                responseInfo["is_heat_mode"] = _parent->setting.isHeatMode;
                responseInfo["default"] = _parent->setting.setTemperature;

                _roomId = roomId;
            }
        }
    }

    return responseInfo;
}

json Dispatcher::StateHandle(json &requestInfo)
{
    static bool preValid = false;
    static time_t beginTime;
    static int beginSpeed;
    static int beginTemp;
    static int beginPower;


    _state.isHeatMode = requestInfo["is_heat_mode"].get<bool>();
    _state.realTemperature = requestInfo["real_temp"].get<int>();
    _state.setTemperature = requestInfo["set_temp"].get<int>();
    _state.speed = requestInfo["speed"].get<int>();

    bool isValid = false;

    // 从送风队列删除
    if(!requestInfo["is_on"].get<bool>()) {
        _parent->StopServe(this);
        _state.isOn = false;
        _record.count++;
    } else {
        if (_parent->setting.isPowerOn && requestInfo["is_on"].get<bool>() &&
                _state.isHeatMode == _parent->setting.isHeatMode) {
            if (_state.isHeatMode && _state.setTemperature > _state.realTemperature)
                isValid = true;
            else if (!_state.isHeatMode && _state.setTemperature < _state.realTemperature)
                isValid = true;
        }

        // 判断送风队列是否空闲
        if (isValid && !_parent->Serve(this))
            isValid = false;


        _state.isOn = isValid;
    }

    if (isValid) {
        if (_state.speed == 1)
            _state.totalPower += _parent->setting.frequence / 1000 * 0.8;
        else if (_state.speed == 2)
            _state.totalPower += _parent->setting.frequence / 1000 * 1.0;
        else
            _state.totalPower += _parent->setting.frequence / 1000 * 1.3;
    }

    if (preValid == false && isValid == true) {
        beginTime = time(nullptr);
        beginSpeed = _state.speed;
        beginTemp = _state.realTemperature;
        beginPower = _state.totalPower;
    } else if (preValid == true && isValid == false) {
        _record.requests.push_back(Request{
                                       beginTime, time(nullptr),
                                       beginTemp, _state.realTemperature,
                                       beginSpeed, _state.speed,
                                       (_state.totalPower - beginPower) * 5
                                   });
    }

    json responseInfo = {
        {"ret", REPLY_CON},
        {"is_valid", isValid},
        {"money", _state.totalPower * 5},
        {"power", _state.totalPower},
        {"frequence", _parent->setting.frequence}
    };

    return responseInfo;
}


void Dispatcher::Logout()
{
    _parent->Offline(_roomId);
}
