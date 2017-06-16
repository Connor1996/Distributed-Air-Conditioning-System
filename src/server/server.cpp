#include "server.h"

#include "../protocol.h"
#include "../include/json.hpp"

using namespace Connor_Socket;
using json = nlohmann::json;

#include "src/model/roominfo.h"
#include "src/model/roomrequest.h"
#include "ormlite.h"

using namespace ORMLite;
#define DATABASE_NAME "data"

Server::Server()
    : setting({false, false, 25, 1, 500}),
      _persistThread(new std::thread([this](){

        while(true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));

            // 使用_dispatchers是为了只检测可能更新的roomId
            for (const auto& pair : _dispatchers) {
                if (!PersistRoomRecord(pair.first))
                    std::cout << "[ERROR] record " << pair.first << " unsuccessful" << std::endl;
            }
        }
      })),
      _count(0)
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

bool Server::PersistRoomRecord(int roomId) {

    static ORMapper<RoomInfo> mapper(DATABASE_NAME);
    RoomInfo helper;
    QueryMessager<RoomInfo> messager(helper);

    std::cout << "[INFO] persist " << roomId << "records" << std::endl;

    auto& record = _rooms[roomId].second;

    // 查询用户背包信息
    auto result = mapper.Query(messager
                              .Where(Field(helper.room_id) == roomId));

    if (result) {
        if (messager.GetVector().size() == 0) {
            helper.count = record.count;
            helper.room_id = roomId;
            std::cout << "[INFO] insert " << roomId << "first record ";
            if (!mapper.Insert(helper))
                return false;
            std::cout << "successful" << std::endl;
        }
        else {
            int count = record.count;
            std::cout << "count: " << count << std::endl;
            helper.count = std::stoi(messager.GetVector()[0][1]) + count;
            helper.room_id = roomId;
            if(mapper.Update(helper))
            // 清除记录数
                record.count -= count;
        }
    } else
        return false;

    while(record.requests.size()) {
        static ORMapper<RoomRequest> requestMapper(DATABASE_NAME);

        struct Request request = record.requests.front();
        RoomRequest info = {
            roomId,
            request.beginTime,
            request.stopTime,
            request.beginTemp,
            request.stopTemp,
            request.beginSpeed,
            request.stopSpeed,
            request.money
        };

        if(!requestMapper.Insert(info))
            return false;
        record.requests.pop_front();
    }
    std::cout << "[INFO] record " << roomId  << " successful" << std::endl;

    return true;
}

std::vector<struct Request> Server::GetRoomRequests(int roomId) {
    ORMapper<RoomRequest> requestMapper(DATABASE_NAME);
    RoomRequest helper;
    QueryMessager<RoomRequest> messager(helper);

    auto result = requestMapper.Query(messager
                                    .Where(Field(helper.room_id) == roomId));

    std::vector<struct Request> ret;
    if (result) {
        for (const auto& request : messager.GetVector()) {
            ret.emplace_back(Request{
                                 std::stoi(request[1]),
                                 std::stoi(request[2]),
                                 std::stoi(request[3]),
                                 std::stoi(request[4]),
                                 std::stoi(request[5]),
                                 std::stoi(request[6]),
                                 std::stod(request[7])
                          });
        }
    }

    return ret;
}

int Server::GetRoomCount(int roomId) {
    ORMapper<RoomInfo> mapper(DATABASE_NAME);
    RoomInfo helper;
    QueryMessager<RoomInfo> messager(helper);

    auto result = mapper.Query(messager
                                    .Where(Field(helper.room_id) == roomId));
    if (result && messager.GetVector().size() != 0)
        return std::stoi(messager.GetVector()[0][1]);
    else
        return 0;
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
                        // 下线处理由dispatcher的析构函数进行
                        cout << "[INFO] Someone offline, now " << --_count << " connections in total" << endl;
                        mtx.unlock();

                        // 断开连接
                        shutdown(connection, SD_BOTH);
                        closesocket(connection);
                        break;
                    }

                    string receiveInfo(recvBuf);
                    receiveInfo = receiveInfo.substr(0, receiveInfo.find("}") + 1);
                    cout << "[INFO] receive message: " << receiveInfo << endl;

                    std::string responseStr;
                    try
                    {
                        // 调用Dispatcher分发到相应处理逻辑
                        responseStr = dispatcher.Dispatch(std::move(json::parse(receiveInfo)));
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

bool Server::Online(int roomId, std::string userId, Dispatcher* connection)
{
    if (_rooms.find(roomId) != _rooms.end())
        if (_rooms[roomId].first == userId) {
            // emplace返回一个pair，第二个元素为是否成功插入
            // 若map中已经有一个同插入相同的key，则不进行插入
            auto result = _dispatchers.emplace(std::make_pair(roomId, connection));
            return result.second;
        }
    return false;
}

void Server::Offline(int roomId)
{
    // 将用户名从在线列表移除
    _dispatchers.erase(roomId);
}

bool Server::CheckIn(int roomId, std::string userId) {
    auto result = _rooms.emplace(std::make_pair(
                                     roomId, std::make_pair(
                                         userId, Record{0, 0, std::list<struct Request>{}}
                                         )
                                     )
                                 );
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
        this->StopServe(_dispatchers[roomId]);
        this->Offline(roomId);

        _rooms.erase(roomId);
        return true;
    }
}

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
        return count <= setting.maxServe;
    else {
        _servingQueue.push_back(target);
        return count + 1 <= setting.maxServe;
    }
}


void Server::StopServe(Dispatcher *target) {
    for (auto iter = _servingQueue.begin(); iter != _servingQueue.end(); ) {
        if (*iter == target)
            iter = _servingQueue.erase(iter);
        else
            iter++;
    }

    std::cout << "[INFO] stop server, still have " << _servingQueue.size() << std::endl;
}


double Server::GetRoomMoney(int roomId) {
    return GetRoomPower(roomId) * 5;
}

double Server::GetRoomPower(int roomId) {
    if (_rooms.find(roomId) != _rooms.end())
        return _rooms[roomId].second.totalPower;
    else
        return 0;
}

