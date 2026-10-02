//
// Created by sergey on 9/29/26.
//

#ifndef PID_REGULATOR_CALIBRATION_LOGGER_H
#define PID_REGULATOR_CALIBRATION_LOGGER_H

#ifndef CALIBRATION_LOGGER_H
#define CALIBRATION_LOGGER_H

#include <stdbool.h>
#include <stdint.h>

bool CalibrationLogger_Init(void);

void CalibrationLogger_Start(void);

void CalibrationLogger_Result(
    int32_t target,
    int32_t average_position,
    int32_t u_centi_percent
);

void CalibrationLogger_Timeout(
    int32_t target,
    int32_t position,
    int32_t u_milli,
    uint32_t duty
);

void CalibrationLogger_Done(void);

#endif /* CALIBRATION_LOGGER_H */

#endif //PID_REGULATOR_CALIBRATION_LOGGER_H
