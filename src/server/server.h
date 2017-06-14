#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>
#include <set>

#include "dispatch.h"
#include "src/socket.h"
#include "request.h"

// 前置声明解决相互包含
class Dispatcher;

struct Setting {
    bool isPowerOn;
    bool isHeatMode;
    int setTemperature;
    int maxServe;
    int frequence;
};


namespace Connor_Socket {

class Server : Socket
{
public:
    // 构造函数，打开监听接口等待请求
    Server();

    ~Server();

    void Start();

    // 用户从控机上线，验证roomId所对应的userId
    // 同时检查是否已经由其他从控机登陆该roomId
    bool Online(int roomId, std::string userId, Dispatcher* connection);
    void Offline(int roomId);

    bool CheckIn(int roomId, std::string userId);
    bool CheckOut(int roomId);

    bool Serve(Dispatcher *);
    void StopServe(Dispatcher *);

    // 获取各种状态
    struct State* GetRoomState(int roomId);
    double GetRoomMoney(int roomId);
    double GetRoomPower(int roomId);
    Record& GetRoomRecord(int roomId) { return _rooms[roomId].second; }
    const std::unordered_map<int, Dispatcher*>& GetRoomMap() { return _dispatchers; }

    // 持久化房间记录
    bool PersistRoomRecord(int);
    //获得房间请求
    std::vector<Request> GetRoomRequests(int);
    //获得房间开关次数
    int GetRoomCount(int);

    Setting setting;

protected:
    // 监听客户端访问的socket
    SOCKET _listeningSocket;

    // 地址信息
    SOCKADDR_IN _serverAddr;

    // 定期持久化线程
    std::thread *_persistThread;

    // 持有与各个客户端保持通信的线程
    std::vector<std::thread> _socketThreads;

    // 持有用户名相对应的dispathcer
    std::unordered_map<int, Dispatcher*> _dispatchers;

    // 房间对应的身份证号以及各项记录
    std::unordered_map<int, std::pair<std::string, struct Record>> _rooms;

    // 所连接的socket数目
    int _count;

    // 送风队列
    std::list<Dispatcher *> _servingQueue;
};

}
#endif // SERVER_H
