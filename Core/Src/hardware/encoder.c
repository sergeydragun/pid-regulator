//
// Created by sergey on 9/29/26.
//

#include "encoder.h"
#include "tim.h"

int32_t Encoder_GetPosition(void)
{
    return (int32_t)__HAL_TIM_GET_COUNTER(&htim2);
}
