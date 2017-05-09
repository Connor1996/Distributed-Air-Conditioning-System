#ifndef PROTOCOL_H
#define PROTOCOL_H

/////////////////////主控机向从控机发送的信息//////////////////////////
#define LOG_IN_SUCC -1
// 主控机确定从控机的登陆请求合法
// int ret = LOG_IN_SUCC

#define LOG_IN_FAIL -2
// 主控机确定从控机的登陆请求不合法
// int ret = LOG_IN_FAIL

#define REPLY_CON -3
// 主控机回复从控机是否送风
// int ret = REPLY_CON
// bool is_vaild ---是否送风
// double power
// double money

///////////////////////从控机向主控机发送的信息/////////////////////////
#define LOG_IN_USER 1
// 从控机输入房间号和身份证号，让主控机验证是否合法
// int op = LOG_IN_USER
// int room_id
// string user_id


#define REPORT_STATE 3
// 从控机回复状态，用于主控机监测房间状态
// 只在开机状态下发送，如关机需要发送一个isOn = false的消息，告知主控机自己关机
// int op = REPORT_STATE
// bool is_on --- 是否开机
// bool is_heat_mode ---工作模式
// int set_temp --- the temperature user wants to get
// int real_temp --- the actual temperature in user's room
// int speed = 1..3


#endif //PROTOCOL_H
