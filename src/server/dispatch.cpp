#include "dispatch.h"
#include "ormlite.h"

#include "../protocol.h"
#include "../model/roomrequest.h"

#include <stdexcept>
#include <algorithm>
#include <unordered_set>
#include <ctime>

using namespace ORMLite;
using std::cout;
#include <QMessageBox>

Dispatcher::~Dispatcher() {
    // 强制持久化
    // 因为周期性持久化只持久化dispatcher在线的roomId
    // 所以dispatcher下线前需要刷新一下距离上次周期持久化后的剩余request
    if (!_server->PersistRoomRecord(_roomId))
        std::cout << "[ERROR] lose some records of " << _roomId << std::endl;
    _server->GetRoomRecord(_roomId).totalPower += _state.totalPower;
    // 下线处理，从server的_dispatchers的map中删除
    _Logout();
}


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

    // 使用用户名及密码尝试登陆
    if (_server->Online(roomId, userId, this)) {
        responseInfo["ret"] = LOG_IN_SUCC;
        responseInfo["is_heat_mode"] = _server->setting.isHeatMode;
        responseInfo["default"] = _server->setting.setTemperature;

        _roomId = roomId;
    }


    return responseInfo;
}

#define LOW_SPEED 0
#define NORMAL_SPEED 1
#define HIGH_SPEED 2
json Dispatcher::StateHandle(json &requestInfo)
{
    static bool preValid = false;
    static time_t beginTime;
    static int beginSpeed;
    static int beginTemp;
    static int beginPower;

    Record& record = _server->GetRoomRecord(_roomId);

    _state.isHeatMode = requestInfo["is_heat_mode"].get<bool>();
    _state.realTemperature = requestInfo["real_temp"].get<int>();
    _state.setTemperature = requestInfo["set_temp"].get<int>();
    _state.speed = requestInfo["speed"].get<int>();

    bool isValid = false;

    // 从送风队列删除
    if(!requestInfo["is_on"].get<bool>()) {
        _server->StopServe(this);
        _state.isOn = false;
        record.count++;
    } else {
        if (_server->setting.isPowerOn && requestInfo["is_on"].get<bool>() &&
                _state.isHeatMode == _server->setting.isHeatMode) {
            if (_state.isHeatMode && _state.setTemperature > _state.realTemperature)
                isValid = true;
            else if (!_state.isHeatMode && _state.setTemperature < _state.realTemperature)
                isValid = true;
        }

        // 判断送风队列是否空闲
        if (isValid && !_server->Serve(this))
            isValid = false;


    }

    if (!isValid)
        _server->StopServe(this);
    _state.isOn = isValid;

    if (isValid) {
        if (_state.speed == LOW_SPEED)
            _state.totalPower += 0.8 * _server->setting.frequence / 1000;
        else if (_state.speed == NORMAL_SPEED)
            _state.totalPower += 1.0 * _server->setting.frequence / 1000;
        else if (_state.speed == HIGH_SPEED)
            _state.totalPower += 1.3 * _server->setting.frequence / 1000;
    }

    if (preValid == false && isValid == true) {
        beginTime = time(nullptr);
        beginSpeed = _state.speed;
        beginTemp = _state.realTemperature;
        beginPower = _state.totalPower;
    } else if (preValid == true && isValid == false) {
        std::cout << "[INFO] push record: " << beginTime << " " << beginTemp << " " << beginSpeed
                  << " " << (_state.totalPower - beginPower) * 5 << std::endl;
        record.requests.push_back(Request{
                                       beginTime, time(nullptr),
                                       beginTemp, _state.realTemperature,
                                       beginSpeed, _state.speed,
                                       (_state.totalPower - beginPower) * 5
                                   });
    }

    preValid = isValid;

    json responseInfo = {
        {"ret", REPLY_CON},
        {"is_valid", isValid},
        {"cost", _state.totalPower * 5},
        {"power", _state.totalPower},
        {"frequence", _server->setting.frequence}
    };

    return responseInfo;
}


void Dispatcher::_Logout()
{
    _server->StopServe(this);
    _server->Offline(_roomId);
}
