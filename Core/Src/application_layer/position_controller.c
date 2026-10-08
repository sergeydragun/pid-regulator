//
// Created by sergey on 10/1/26.
//
#include "global_funcs.h"
#include "global_vars.h"
#include "control_layer/adrc.h"
#include "hardware_layer/motor.h"

#define POSITION_CONTROLLER_KP            1.0f
#define POSITION_CONTROLLER_KD            1.0f

#define POSITION_CONTROLLER_DT            0.020f

#define POSITION_CONTROLLER_OUTPUT_MIN   (-100.0f)
#define POSITION_CONTROLLER_OUTPUT_MAX     100.0f

void PositionController_Init(int32_t initial_pos)
{
    float dt = 0.010f; // 10 мс
    float wc = 10.0f;  // Полоса регулятора
    float wo = 40.0f;  // Полоса наблюдателя
    float b  = 1.0f;   // Оценка коэффициента передачи мотора

    float kp = wc * wc;
    float kd = 2.0f * wc;

    ADRC_Init(&g_adrc, wo, b, kp, kd, dt, initial_pos);
    Motor_Init();
}

float PositionController_Update(float target, int32_t position)
{
    float u_fb = ADRC_Update(&g_adrc, target, position);

    const float u =
        u_fb;

    return clamp(
        u,
        POSITION_CONTROLLER_OUTPUT_MIN,
        POSITION_CONTROLLER_OUTPUT_MAX
    );
}
