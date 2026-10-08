//
// Created by sergey on 10/1/26.
//

#ifndef PID_REGULATOR_POSITION_CONTROLLER_H
#define PID_REGULATOR_POSITION_CONTROLLER_H
#include <stdint.h>

void PositionController_Init(float initial_pos);

float PositionController_Update(float target, float position);

#endif //PID_REGULATOR_POSITION_CONTROLLER_H
