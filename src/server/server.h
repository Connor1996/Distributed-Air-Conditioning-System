#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>
#include <set>

#include "dispatch.h"
#include "../socket.h"
// 前置声明解决相互包含
class Dispatcher;

struct Setting {
    bool isHeatMode;
    int setTemperature;
};

namespace Connor_Socket {

class Server : Socket
{
    friend class Dispatcher;
public:
    // 构造函数，打开监听接口等待请求
    Server();

    ~Server();

    bool Online(int roomId, Dispatcher* connection);

    void Offline(int roomId);

    bool CheckIn(int roomId, std::string userId);

    bool CheckOut(int roomId);

    struct Setting GetSetting() { return _setting; }

protected:
    // 监听客户端访问的socket
    SOCKET _listeningSocket;

    // 地址信息
    SOCKADDR_IN _serverAddr;

    // 持有与各个客户端保持通信的线程
    std::vector<std::thread> _socketThreads;

    // 持有用户名相对应的dispathcer
    std::unordered_map<int, Dispatcher*> _sockets;

    std::unordered_map<int, std::string> _rooms;

    int _count;

    Setting _setting;
};

}
#endif // SERVER_H
