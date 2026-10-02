//
// Created by sergey on 10/1/26.
//

#include "position_controller.h"
#include "calibration/static_compensation.h"
#include "pid_calculator.h"
#include "global_funcs.h"

#define POSITION_CONTROLLER_KP            6.0f
#define POSITION_CONTROLLER_KI            0.0f
#define POSITION_CONTROLLER_KD            0.0f

#define POSITION_CONTROLLER_DT            0.020f

#define POSITION_CONTROLLER_OUTPUT_MIN   (-100.0f)
#define POSITION_CONTROLLER_OUTPUT_MAX     100.0f

void PositionController_init(void)
{
    PID_Init(
        POSITION_CONTROLLER_KP,
        POSITION_CONTROLLER_KI,
        POSITION_CONTROLLER_KD,
        POSITION_CONTROLLER_DT,
        POSITION_CONTROLLER_OUTPUT_MIN,
        POSITION_CONTROLLER_OUTPUT_MAX
    );

    init_compensation_table();
}

float PositionController_update(int32_t target, int32_t position)
{
    float u_fb = PID_Update(target, position);
    float u_stat = StaticCompensation_Get(target);

    const float u =
        u_fb +
        u_stat;

    return clamp(
        u,
        POSITION_CONTROLLER_OUTPUT_MIN,
        POSITION_CONTROLLER_OUTPUT_MAX
    );
}
