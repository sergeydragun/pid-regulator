//
// Created by sergey on 9/28/26.
//

#ifndef PID_REGULATOR_PID_CALCULATOR_H
#define PID_REGULATOR_PID_CALCULATOR_H
#include <stdint.h>

void PID_Init(float kp,
              float ki,
              float kd,
              float dt,
              float output_min,
              float output_max);

float PID_Update(int32_t target, int32_t measurement);

void PID_Reset(void);

#endif //PID_REGULATOR_PID_CALCULATOR_H
