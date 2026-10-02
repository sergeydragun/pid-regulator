#include "pid_calculator.h"

#include <stdint.h>

typedef struct
{
    float kp;
    float ki;
    float kd;

    float prev;
    float integral;

    float dt;

    float output_min;
    float output_max;
} PIDData;

static PIDData pid;


void PID_Init(float kp,
              float ki,
              float kd,
              float dt,
              float output_min,
              float output_max)
{
    pid.kp = kp;
    pid.ki = ki;
    pid.kd = kd;

    pid.dt = dt;

    pid.output_min = output_min;
    pid.output_max = output_max;

    PID_Reset();
}


float PID_Update(int32_t target, int32_t measurement)
{
    const float error = target - measurement;

    float new_integral =
        pid.integral + error * pid.dt;

    const float derivative =
        (error - pid.prev) / pid.dt;

    float output =
        pid.kp * error
        + pid.ki * new_integral
        + pid.kd * derivative;

    if (output > pid.output_max)
    {
        output = pid.output_max;

        if (error < 0.0f)
        {
            pid.integral = new_integral;
        }
    }
    else if (output < pid.output_min)
    {
        output = pid.output_min;

        if (error > 0.0f)
        {
            pid.integral = new_integral;
        }
    }
    else
    {
        pid.integral = new_integral;
    }

    pid.prev = error;

    return output;
}


void PID_Reset(void)
{
    pid.prev = 0.0f;
    pid.integral = 0.0f;
}