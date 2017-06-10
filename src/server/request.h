#ifndef REQUEST_H
#define REQUEST_H

struct Request {
    time_t beginTime;
    time_t stopTime;
    int beginTemp;
    int stopTemp;
    int beginSpeed;
    int stopSpeed;
    double money;
};

struct Record {
    int count;      // 开关机次数
    double totalPower;
    std::list<struct Request> requests;
};

#endif // REQUEST_H
