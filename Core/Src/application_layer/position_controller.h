//
// Created by sergey on 10/1/26.
//

#ifndef PID_REGULATOR_POSITION_CONTROLLER_H
#define PID_REGULATOR_POSITION_CONTROLLER_H
#include <stdint.h>

// Период контура. Наблюдателю нужен быстрый опрос: при 10 мс полоса ESO
// упирается в ~50 рад/с, а объект (трение, пружина) меняется быстрее.
#define POSITION_CONTROLLER_PERIOD_MS     2U

void PositionController_Init(void);

float PositionController_Update(float target, int32_t position);

#endif //PID_REGULATOR_POSITION_CONTROLLER_H
