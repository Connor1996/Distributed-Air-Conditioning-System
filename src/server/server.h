#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>
#include <set>

#include "dispatch.h"
#include "../socket.h"
// 前置声明解决相互包含
class Dispatcher;

struct Setting {
    bool isPowerOn;
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

    void Start();

    bool Online(int roomId, Dispatcher* connection);

    void Offline(int roomId);

    bool CheckIn(int roomId, std::string userId);

    bool CheckOut(int roomId);

    bool Serve(Dispatcher *);

    void StopServe(Dispatcher *);

    //const struct Setting& GetSetting() { return _setting; }
    struct State* GetRoomState(int roomId);
    const std::unordered_map<int, Dispatcher*>& GetRoomMap() {
        return _dispatchers;
    }

    Setting _setting;
protected:
    // 监听客户端访问的socket
    SOCKET _listeningSocket;

    // 地址信息
    SOCKADDR_IN _serverAddr;

    // 持有与各个客户端保持通信的线程
    std::vector<std::thread> _socketThreads;

    // 持有用户名相对应的dispathcer
    std::unordered_map<int, Dispatcher*> _dispatchers;

    // 房间对应的身份证号
    std::unordered_map<int, std::string> _rooms;

    // 所连接的socket数目
    int _count;

    // 送风队列
    std::list<Dispatcher *> _servingQueue;
};

}
#endif // SERVER_H
