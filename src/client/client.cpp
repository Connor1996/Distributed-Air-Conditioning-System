#include "client.h"
#include "src/include/json.hpp"

using namespace Connor_Socket;

Client::Client(string name, string ipaddr, int port)
    : _username(name),
      _ipaddr(ipaddr),
      _port(port)
{
    std::cout << "begin..." << std::endl;
    _connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_connectSocket == -1)
        throw std::runtime_error("Cannot create socket");

    // 填写客户端地址信息
    // 使用inet_addr需要将IP地址转换为网络格式
    _serverAddr.sin_family = AF_INET;
    //_serverAddr.sin_port = htons(SERVER_PORT);
    _serverAddr.sin_port = htons(_port);
    //_serverAddr.sin_addr.s_addr = inet_addr(CLIENT_ADDR);
    //auto res = inet_pton(AF_INET, SERVER_ADDR, &_serverAddr.sin_addr);
    auto res = inet_pton(AF_INET, _ipaddr.c_str(), &_serverAddr.sin_addr);
    if (res < 0)
    {
        closesocket(_connectSocket);
        throw std::runtime_error("First parameter is not a valid address family");
    }
    else if (res == 0)
    {
        closesocket(_connectSocket);
        throw std::runtime_error("Second parrameter does not contain valid ipaddress");
    }

    //设置发送时限
    //int nNetTimeout = 1000;
    //setsockopt(_connectSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout, sizeof(int));

    cout << "Initial success" << endl;
}

Client::~Client()
{
    shutdown(_connectSocket, SD_BOTH);
    closesocket(_connectSocket);
}

void Client::Close()
{
    closesocket(_connectSocket);
}

std::string Client::Connect(std::string requestInfo)
{
    auto trys = 1;
    auto success = false;

    // 多次尝试
    while(trys--)
    {
        //与服务器建立连接
        if (connect(_connectSocket, reinterpret_cast<SOCKADDR *>(&_serverAddr),
                    sizeof(_serverAddr))== -1)
        {
             cout << "Unable to connect to server... " << std::to_string(2 - trys) << " trys" << endl;
             std::this_thread::sleep_for(std::chrono::milliseconds(10));
             continue;
        }
        cout << "Connect success" << endl;
        success = true;
        break;
    }
    if (!success)
    {
        closesocket(_connectSocket);
        throw std::runtime_error("Unable to connect to server, Please try later");
    }

    // 连接成功建立后，发送请求
    Send(requestInfo);

    return std::move(Receive());
}

void Client::Send(std::string requestInfo)
{
    std::lock_guard<std::mutex> guard(mtx);

    cout << "[INFO] request: " << requestInfo << endl;

    char sendBuf[DEFAULT_BUFLEN];
    int sendBufLen = DEFAULT_BUFLEN;
    strcpy(sendBuf, requestInfo.c_str());


    // 向服务器发送数据
    if (send(_connectSocket, sendBuf, sendBufLen, 0) == SOCKET_ERROR)
    {
        closesocket(_connectSocket);
        throw std::runtime_error("Failed at send message");
    }
    cout << "[INFO] send complete" << endl;

}

std::string Client::Receive()
{
    int recvBufLen = DEFAULT_BUFLEN;
    char recvBuf[DEFAULT_BUFLEN];

    // 等待接受服务器的返回信息
    if (recv(_connectSocket, recvBuf, recvBufLen, 0) <= 0)
    {
        closesocket(_connectSocket);
        throw std::runtime_error("Failed at receive message");
    }

    string receiveInfo(recvBuf);
    receiveInfo = receiveInfo.substr(0, receiveInfo.find("}") + 1);
    cout << "[INFO] receive: " << receiveInfo << endl;

    return receiveInfo;
}
