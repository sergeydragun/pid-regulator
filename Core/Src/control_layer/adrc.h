//
// Created by sergey on 10/2/26.
//

#ifndef PID_REGULATOR_ADRC_H
#define PID_REGULATOR_ADRC_H
#include <stdint.h>

typedef struct
{
    float wo;
    float b;
    float kp;
    float kd;
    float dt;

    float x1_hat;
    float x2_hat;
    float x3_hat;

    float out_min;
    float out_max;

    float beta1;
    float beta2;
    float beta3;
} ADRC_t;

void ADRC_Init(volatile ADRC_t *adrc, float wo, float b, float kp, float kd, float dt, int32_t initial_pos);
float ADRC_Update(volatile ADRC_t *adrc, float target, int32_t current_pos);

#endif //PID_REGULATOR_ADRC_H
