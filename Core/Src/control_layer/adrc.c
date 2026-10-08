//
// Created by sergey on 10/2/26.
//

#include "adrc.h"

#include "global_funcs.h"
#include "logger/logger.h"

void ADRC_Init(volatile ADRC_t* adrc, float wo, float b, float kp, float kd, float dt,
               float out_limit, int32_t initial_pos)
{
    adrc->wo = wo;
    adrc->b = b;
    adrc->kp = kp;
    adrc->kd = kd;
    adrc->dt = dt;

    adrc->x1_hat = (float)initial_pos;
    adrc->x2_hat = 0.0f;
    adrc->x3_hat = 0.0f;
    adrc->u_prev = 0.0f;

    adrc->out_min = -out_limit;
    adrc->out_max = out_limit;

    adrc->beta1 = 3 * wo;
    adrc->beta2 = 3 * wo * wo;
    adrc->beta3 = wo * wo * wo;

    adrc->log_every = (uint32_t)(0.010f / dt + 0.5f);
    if (adrc->log_every == 0)
        adrc->log_every = 1;
    adrc->log_counter = 0;
}

float ADRC_Update(volatile ADRC_t* adrc, float target, int32_t current_pos)
{
    // 1. Наблюдатель: сначала догоняем измерение, используя u, которое
    //    действовало на объект весь прошедший шаг.
    const float e = adrc->x1_hat - (float)current_pos;

    adrc->x1_hat = adrc->x1_hat + (adrc->x2_hat - adrc->beta1 * e) * adrc->dt;
    adrc->x2_hat = adrc->x2_hat + (adrc->x3_hat + adrc->b * adrc->u_prev - adrc->beta2 * e) * adrc->dt;
    adrc->x3_hat = adrc->x3_hat - (adrc->beta3 * e) * adrc->dt;

    // 2. Закон управления по свежим оценкам.
    const float u_0 = adrc->kp * (target - adrc->x1_hat) - adrc->kd * adrc->x2_hat;
    const float u = clamp((u_0 - adrc->x3_hat) / adrc->b, adrc->out_min, adrc->out_max);

    adrc->u_prev = u;

    if (++adrc->log_counter < adrc->log_every)
        return u;
    adrc->log_counter = 0;

    // x3 выводим в единицах управления (%), чтобы было сравнимо с u.
    const float x3_u = adrc->x3_hat / adrc->b;

    Log_Write("%s%d.%02d|%s%d.%02d|%s%d.%02d|%s%d.%02d|%s%d.%02d|%s%d.%02d",
        FMT_F(target),
        FMT_F((float)current_pos),
        FMT_F(adrc->x1_hat),
        FMT_F(adrc->x2_hat),
        FMT_F(x3_u),
        FMT_F(u));

    return u;
}
