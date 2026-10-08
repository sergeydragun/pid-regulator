//
// Created by sergey on 10/2/26.
//

#include "adrc.h"

#include "global_funcs.h"
#include "logger/logger.h"

void ADRC_Init(volatile ADRC_t* adrc, float wo, float b, float kp, float kd, float dt, int32_t initial_pos)
{
    adrc->wo = wo;
    adrc->b = b;
    adrc->kp = kp;
    adrc->kd = kd;
    adrc->dt = dt;
    adrc->x1_hat = initial_pos;

    adrc->out_min = 0;
    adrc->out_max = 100;

    adrc->beta1 = 3 * wo;
    adrc->beta2 = 3 * wo * wo;
    adrc->beta3 = wo * wo * wo;
}

float ADRC_Update(volatile ADRC_t* adrc, float target, int32_t current_pos)
{
    float e = adrc->x1_hat - current_pos;

    float u_0 = adrc->kp * (target - adrc->x1_hat) - adrc->kd * adrc->x2_hat;
    float u = (u_0 - adrc->x3_hat) / adrc->b;

    adrc->x1_hat = adrc->x1_hat + (adrc->x2_hat - adrc->beta1 * e) * adrc->dt;
    adrc->x2_hat = adrc->x2_hat + (adrc->x3_hat + adrc->b * u - adrc->beta2 * e) * adrc->dt;
    adrc->x3_hat = adrc->x3_hat - (adrc->beta3 * e) * adrc->dt;

    Log_Write("%d.%02d|%d.%02d|%d.%02d|%d.%02d|%d.%02d|%d.%02d",
        FMT_F(target),
        FMT_F(current_pos),
        FMT_F(adrc->x1_hat),
        FMT_F(adrc->x2_hat),
        FMT_F(adrc->x3_hat),
        FMT_F(u));

    return clamp(u, adrc->out_min, adrc->out_max);
}
