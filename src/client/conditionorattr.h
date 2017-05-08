#ifndef CONDITIONORATTR_H
#define CONDITIONORATTR_H

enum Switch {OFF = 0, ON = 1};
enum TempRange {LOWER_BOUND = -30, UPPER_BOUND = 30};
enum Speed {SLOW_SPEED = 0, NORMAL_SPEED = 1, FAST_SPEED = 2};
const int TempInc[3] = {1, 2, 3}; //每秒变化的温度
const std::string SpeedStr[3] = {"Slow", "Normal", "Fast"};
const int TEMP_CHANGE_CIRCUIT = 3000; //ms
const int TEMP_BEAR_RANGE = 5; //可忍受的温度恢复程度

typedef struct conditionorAttr {
    Switch onoff;
    bool is_heat_mode;
    Speed speed;
    int temp;
    int expTemp;
    int original_temp;
} ConditionorAttr;

#endif // CONDITIONORATTR_H
