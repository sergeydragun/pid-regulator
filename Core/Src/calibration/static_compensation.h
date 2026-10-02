//
// Created by sergey on 10/1/26.
//

#ifndef PID_REGULATOR_STATIC_COMPENSATION_H
#define PID_REGULATOR_STATIC_COMPENSATION_H
#include <stdbool.h>
#include <stdint.h>

#define STATIC_COMPENSATION_STEP 3

void init_compensation_table();

void StaticCompensation_Set(
    int32_t position,
    float u_stat
    );

float StaticCompensation_Get(int32_t position);

void StaticCompensation_Reset();

#endif //PID_REGULATOR_STATIC_COMPENSATION_H
