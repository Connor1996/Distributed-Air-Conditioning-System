#ifndef DISPATCH_H
#define DISPATCH_H

#include "server.h"
#include "src/socket.h"
#include "src/include/json.hpp"

using json = nlohmann::json;

// 前置声明解决相互包含
namespace Connor_Socket {
    class Server;
}

struct State {
    bool isOn;
    bool isHeatMode;
    int setTemperature;
    int realTemperature;
    int speed;
    double totalPower;
};


class Dispatcher
{
public:

    // 传入SOCKET和Server的构造函数
    // @param:
    //      connection  与相应客户端建立的socket连接
    //      parent      server对象指针
    Dispatcher(SOCKET &connection, Connor_Socket::Server *server)
        : _connection(connection), _server(server),
          _state({false, false, 0, 0, 1, 0})
    { }

    ~Dispatcher();

    // 根据请求信息，分发到相应的函数处理请求
    // @param:
    //      requestInfo json序列化后请求信息
    // @return:
    //      json序列化后的返回信息
    std::string Dispatch(json requestInfo);

    // 登陆处理逻辑
    json LoginHandle(json&);
    json StateHandle(json&);

    // 获取Dispatcher的内部状态
    struct State* GetState() { return &_state; }

    double GetPower() const { return _state.totalPower; }
    double GetMoney() const { return _state.totalPower * 5; }


private:
    // 房间状态
    State _state;

    // 与dispatcher绑定的socket连接
    SOCKET _connection;

    // 与socket连接绑定的房间号
    int _roomId;

    // server指针，以访问server维护的在线列表
    Connor_Socket::Server *_server;

    // 与该Dispatch绑定用户登出
    void _Logout();

};

#endif // DISPATCH_H
