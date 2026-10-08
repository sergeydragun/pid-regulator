//
// Created by sergey on 10/1/26.
//

#ifndef PID_REGULATOR_GLOBAL_FUNCS_H
#define PID_REGULATOR_GLOBAL_FUNCS_H

// Вывод float без printf("%f"): формат "%s%d.%02d", знак отдельно,
// иначе значения из (-1; 0) печатались бы без минуса.
#define FMT_F(x)  ((x) < 0 ? "-" : ""), \
                  (int)((x) < 0 ? -(x) : (x)), \
                  (int)(((x) < 0 ? -(x) : (x)) * 100) % 100

float clamp(
    float value,
    float min,
    float max
);

#endif //PID_REGULATOR_GLOBAL_FUNCS_H
