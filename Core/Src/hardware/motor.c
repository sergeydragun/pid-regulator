//
// Created by sergey on 9/29/26.
//

#include "main.h"
#include "motor.h"

#include <stdio.h>


#include "tim.h"

static uint32_t output_to_ccr(float output)
{
    if (output < 0.0f)
    {
        output = -output;
    }

    if (output > 100.0f)
    {
        output = 100.0f;
    }

    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim1);

    return (uint32_t)((output / 100.0f) * (float)(arr + 1U));
}

void Motor_Init(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

    Motor_SetOutput(0.0f);
}

void Motor_SetOutput(float output)
{
    uint32_t duty = output_to_ccr(output);

    if (output > 0.0f)
    {
        __HAL_TIM_SET_COMPARE(
            &htim1,
            TIM_CHANNEL_1,
            duty
        );

        __HAL_TIM_SET_COMPARE(
            &htim1,
            TIM_CHANNEL_2,
            0
        );
    }
    else if (output < 0.0f)
    {
        __HAL_TIM_SET_COMPARE(
            &htim1,
            TIM_CHANNEL_1,
            0
        );

        __HAL_TIM_SET_COMPARE(
            &htim1,
            TIM_CHANNEL_2,
            duty
        );
    }
    else
    {
        __HAL_TIM_SET_COMPARE(
            &htim1,
            TIM_CHANNEL_1,
            0
        );

        __HAL_TIM_SET_COMPARE(
            &htim1,
            TIM_CHANNEL_2,
            0
        );
    }
}
