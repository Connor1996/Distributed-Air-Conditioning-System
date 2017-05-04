#ifndef CONDITIONORATTR_H
#define CONDITIONORATTR_H

enum TempRange {LOWER_BOUND = -30, UPPER_BOUND = 30};
enum Speed {SLOW_SPEED = 0, NORMAL_SPEED = 1, FAST_SPEED = 2};
const int TempInc[3] = {1, 2, 3}; //每秒变化的温度
const std::string SpeedStr[3] = {"Slow", "Normal", "Fast"};
const int TEMP_CHANGE_CIRCUIT = 3000; //ms

typedef struct conditionorAttr {
    bool is_heat_mode;
    int temp;
    int expTemp;
    Speed speed;
} ConditionorAttr;

#endif // CONDITIONORATTR_H
