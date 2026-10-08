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

    // Управление, реально поданное на прошлом шаге (после ограничения).
    // Наблюдатель должен видеть именно его, иначе x3 улетает при насыщении.
    float u_prev;

    float out_min;
    float out_max;

    float beta1;
    float beta2;
    float beta3;

    // Лог пишется раз в log_every шагов (~10 мс), UART не успевает чаще.
    uint32_t log_every;
    uint32_t log_counter;
} ADRC_t;

void ADRC_Init(volatile ADRC_t *adrc, float wo, float b, float kp, float kd, float dt,
               float out_limit, int32_t initial_pos);
float ADRC_Update(volatile ADRC_t *adrc, float target, int32_t current_pos);

#endif //PID_REGULATOR_ADRC_H
