#ifndef CLIENT_H
#define CLIENT_H

#include "../socket.h"
#include <mutex>

const std::string DEFAULT_SERVER_IP = "127.0.0.1";
const int DEFAULT_SERVER_PORT = 6666;

namespace Connor_Socket {

class Client : Socket{
public:
    // 默认构造函数
    // 委托到带名字的构造函数，传参为UNKOWN
    Client() : Client("UNKNOWN", SERVER_ADDR, SERVER_PORT) { }

    // 带名字的构造函数，表明与该socket绑定的用户名
    // @param:
    //      name 用户名
    Client(string name, string ipaddr, int port);

    ~Client();

    // 注册、登陆时使用的函数
    // 建立与服务器的连接
    // @param:
    //      requestInfo json序列化后请求信息
    // @return:
    //      json序列化后的返回信息
    std::string Connect(std::string requestInfo);

    // 建立连接后使用此函数向服务器发送数据
    // @param:
    //      requestInfo json序列化后请求信息
    void Send(std::string requestInfo);

    // 建立连接后使用此函数接收数据
    // @param:
    //      requestInfo json序列化后请求信息
    std::string Receive();

    // 关闭与服务器的连接
    void Close();

    // 获取该client绑定的用户名
    // @return:
    //      用户名
    std::string GetUserName() { return _username; }

protected:
    // 与服务器通信的socket
    SOCKET _connectSocket;
    // 服务器的地址信息
    SOCKADDR_IN _serverAddr;
    // 用户名
    string _username;
    string _ipaddr;
    int _port;
private:
    std::mutex mtx;
};

}
#endif // CLIENT_H
