//
// Created by sergey on 10/1/26.
//

#ifndef PID_REGULATOR_GLOBAL_FUNCS_H
#define PID_REGULATOR_GLOBAL_FUNCS_H

#define FMT_F(x)  (int)(x), (int)(((x) - (int)(x)) * 100 < 0 ? -((x) - (int)(x)) * 100 : ((x) - (int)(x)) * 100)

float clamp(
    float value,
    float min,
    float max
);

#endif //PID_REGULATOR_GLOBAL_FUNCS_H
