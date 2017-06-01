#include "server.h"

#include "../protocol.h"
#include "../include/json.hpp"

using namespace Connor_Socket;
using json = nlohmann::json;


Server::Server() : _setting({false, false, 25}), _count(0)
{
    _listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_listeningSocket == -1)
        throw std::runtime_error("Cannot create listening socket");

    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_port = htons(SERVER_PORT);
    _serverAddr.sin_addr.s_addr = INADDR_ANY;

    // 绑定监听端口
    if (bind(_listeningSocket, reinterpret_cast<SOCKADDR*>(&_serverAddr),
             sizeof(_serverAddr)) == -1)
    {
        closesocket(_listeningSocket);
        throw std::runtime_error("Failed at bind");
    }

    // 开始监听，指定监听队列的大小
    if (listen(_listeningSocket, 5))
    {
        closesocket(_listeningSocket);
        throw std::runtime_error("Failed at listen");
    }
    std::cout << "Wait for connection..." << std::endl;

}

Server::~Server()
{
    shutdown(_listeningSocket, SD_BOTH);
    closesocket(_listeningSocket);
    std::cout << "Server is closed" << std::endl;
}

void Server::Start()
{

    //各个连接客户端线程输出信息锁
    std::mutex mtx;
    // 不断等待客户端请求的到来
    while (true)
    {
        SOCKADDR_IN clientAddr;
        int addrlen = sizeof(clientAddr);
        // 监听到客户端的请求，新建立一个socket保持通信
        SOCKET newConnection = accept(_listeningSocket, (SOCKADDR *)&clientAddr, &addrlen);
        // 将新建的socket放入线程单独运行
        _socketThreads.emplace_back(std::thread([this, &mtx] (SOCKET &&connection)
        {
            char recvBuf[DEFAULT_BUFLEN];
            int recvBufLen = DEFAULT_BUFLEN;
            Dispatcher dispatcher(connection, this);

            if (connection < 0)
            {
                cout << "[ERROR] Failed at accept" << endl;
            }
            else
            {
                cout << "[INFO] New come, now " << ++_count << " connections in total" << endl;

                while(true)
                {
                    if (recv(connection, recvBuf, recvBufLen, 0) <= 0)
                    {
                        // 保证cout完整执行而不被其他线程打断
                        mtx.lock();
                        // 若是在线状态，下线处理
                        dispatcher.Logout();
                        cout << "[INFO] Someone offline, now " << --_count << " connections in total" << endl;
                        mtx.unlock();

                        // 断开连接
                        shutdown(connection, SD_BOTH);
                        closesocket(connection);
                        break;
                    }
                    cout << "[INFO] receive message: " << recvBuf << endl;

                    std::string responseStr;
                    try
                    {
                        // 调用Dispatcher分发到相应处理逻辑
                        responseStr = dispatcher.Dispatch(std::move(json::parse(recvBuf)));
                    }
                    catch (std::exception e)
                    {
                        //shutdown(connection, SD_BOTH);
                        //closesocket(connection);
                        cout << "[ERROR] wrong field for json" << endl;
                    }

                    char recvBuf[DEFAULT_BUFLEN];
                    int recvBufLen = DEFAULT_BUFLEN;
                    strcpy(recvBuf, responseStr.c_str());
                    if (send(connection, recvBuf, recvBufLen, 0) <= 0)
                    {
                        cout << "[ERROR] failed at send messager, code: " << WSAGetLastError() << endl;
                    }
                    else
                        cout << "[INFO] send message: " << responseStr << endl;
                }
            }
        }, newConnection));

        _socketThreads.back().detach(); //使子线程独立运行
    }
}

bool Server::Online(int roomId, Dispatcher* connection)
{
    // emplace返回一个pair，第二个元素为是否成功插入
    // 若map中已经有一个同插入相同的key，则不进行插入
    auto result = _dispatchers.emplace(std::make_pair(roomId, connection));
    return result.second;
}

void Server::Offline(int roomId)
{
    // 将用户名从在线列表移除
    _dispatchers.erase(roomId);
}

bool Server::CheckIn(int roomId, std::string userId) {
    auto result = _rooms.emplace(std::make_pair(roomId, userId));
    return result.second;
}

struct State* Server::GetRoomState(int roomId) {
    if (_dispatchers.find(roomId) != _dispatchers.end())
        return _dispatchers[roomId]->GetState();
    else
        return nullptr;
}

bool Server::CheckOut(int roomId) {
    if (_rooms.find(roomId) == _rooms.end())
        return false;
    else {
        _rooms.erase(roomId);
        return true;
    }
}
#define SERVE_NUMBER 3
bool Server::Serve(Dispatcher *target) {
    bool has = false;
    int count = 0;
    for (const auto& serving : _servingQueue) {
        count++;
        if (serving == target) {
            has = true;
            break;
        }
    }

    if (has)
        return count <= SERVE_NUMBER;
    else {
        _servingQueue.push_back(target);
        return count + 1 <= SERVE_NUMBER;
    }
}


void Server::StopServe(Dispatcher *target) {
    for (auto iter = _servingQueue.begin(); iter != _servingQueue.end(); ) {
        if (*iter == target)
            iter = _servingQueue.erase(iter);
        else
            iter++;
    }
}
