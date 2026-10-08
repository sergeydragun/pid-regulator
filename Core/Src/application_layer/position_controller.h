//
// Created by sergey on 10/1/26.
//

#ifndef PID_REGULATOR_POSITION_CONTROLLER_H
#define PID_REGULATOR_POSITION_CONTROLLER_H
#include <stdint.h>

void PositionController_Init(int32_t initial_pos);

float PositionController_Update(float target, int32_t position);

#endif //PID_REGULATOR_POSITION_CONTROLLER_H
