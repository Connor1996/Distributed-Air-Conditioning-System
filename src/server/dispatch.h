#ifndef DISPATCH_H
#define DISPATCH_H

#include "server.h"
#include "../socket.h"
#include "../include/json.hpp"

using json = nlohmann::json;

// 前置声明解决相互包含
namespace Connor_Socket {
    class Server;
}

struct State {
    bool isOn;
    bool isHeatMode;
    int temperature;
    int speed;
};

class Dispatcher
{
public:

    Dispatcher() : _state({false, false, 0, 0}) { }

    // 传入SOCKET和Server的构造函数
    // @param:
    //      connection  与相应客户端建立的socket连接
    //      parent      server对象指针
    Dispatcher(SOCKET &connection, Connor_Socket::Server *parent)
        : _connection(connection), _parent(parent), _state({false, false, 0, 0})
    {
        // 初始化随机种子
        srand((unsigned int)time(NULL));
    }

    // 根据请求信息，分发到相应的函数处理请求
    // @param:
    //      requestInfo json序列化后请求信息
    // @return:
    //      json序列化后的返回信息
    std::string Dispatch(json requestInfo);

    // 登陆处理逻辑
    json LoginHandle(json&);
    json UpdateSettingHandle(json&);
    json StateHandle(json&);

    // 与该Dispatch绑定用户登出
    void Logout();

    // 获取Dispatcher的内部状态
    struct State getState() { return _state; }
private:
    // 代表用户处于什么状态，如在线空闲或对战
    struct State _state;

    // 与dispatcher绑定的socket连接
    SOCKET _connection;

    // 与socket连接绑定的用户名
    std::string _username;

    // server指针，以访问server维护的在线列表
    Connor_Socket::Server *_parent;

};

#endif // DISPATCH_H
