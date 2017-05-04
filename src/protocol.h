#ifndef PROTOCOL_H
#define PROTOCOL_H

/////////////////////主控机向从控机发送的信息//////////////////////////
#define LOG_IN_SUCC -1
// 主控机确定从控机的登陆请求合法
// int ret = LOG_IN_SUCC
// bool is_heat_mode
// int temp

#define LOG_IN_FAIL -2
// 主控机确定从控机的登陆请求不合法
// int ret = LOG_IN_FAIL

#define REPLY_MONEY -3
//主控机收到从控机发送的状态后，返回房间计费情况
// int ret = REPLY_MONEY
// double cost --- 实时累计金额
// double power --- 实时累计功率

#define REPLY_CON -4;
// 主控机回复从控机是否送风
// int ret = REPLY_CON
// bool is_vaild

///////////////////////从控机向主控机发送的信息/////////////////////////
#define LOG_IN_USER 1
// 从控机输入房间号和身份证号，让主控机验证是否合法
// int op = LOG_IN_USER
// int room_id
// string user_id

#define REQ_STOP 2
// 从控机请求停止送风
// int op = REQ_STOP

#define REQ_RESUME 3
// 从控机请求继续送风
// int op = REQ_RESUME

#define REQ_UPDATE 4
// 从控机更改设置：工作模式、设定温度、风速大小
// int op = REQ_UPDATE
// bool is_heat_mode
// int temp
// int speed = 1..3

#define REPORT_STATE 5
// 从控机周期性回复状态，用于主控机监测房间状态
// int op = REPORT_STATE
// int set_temp --- the temperature user wants to get
// int real_temp --- the actual temperature in user's room
// int speed = 1..3


#endif PROTOCOL_H
