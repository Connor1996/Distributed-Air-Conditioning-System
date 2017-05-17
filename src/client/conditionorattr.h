#ifndef CONDITIONORATTR_H
#define CONDITIONORATTR_H

enum TempRange {LOWER_BOUND = 16, UPPER_BOUND = 30};
enum Speed {SLOW_SPEED = 0, NORMAL_SPEED = 1, FAST_SPEED = 2};
const int TempInc[3] = {1, 2, 3}; //每秒变化的温度
const std::wstring SpeedStr[3] = {L"慢", L"正常", L"快"};
const int RotateRatio[3] = {1, 4, 7};
const int TEMP_CHANGE_CIRCUIT = 3000; //ms
const int TEMP_BEAR_RANGE = 1; //可忍受的温度恢复程度

typedef struct conditionorAttr {
    bool is_on;
    bool is_heat_mode;
    Speed speed;
    int temp;
    int expTemp;
    int original_temp;
} ConditionorAttr;

#endif // CONDITIONORATTR_H
