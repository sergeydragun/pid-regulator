//
// Created by sergey on 9/28/26.
//

#ifndef PID_REGULATOR_GLOBAL_VARS_H
#define PID_REGULATOR_GLOBAL_VARS_H
#include <stdint.h>

#include "control_layer/adrc.h"

extern volatile int32_t g_max_pos;
extern volatile int32_t g_min_pos;

extern volatile float g_target;

extern volatile ADRC_t*  g_adrc;

#endif //PID_REGULATOR_GLOBAL_VARS_H
