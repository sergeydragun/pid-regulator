//
// Created by sergey on 9/28/26.
//

#include "pid_calculator.h"

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

void PID_Init(float kp, float ki, float kd, float dt, float output_min, float output_max)
{
    pid.kp = kp;
    pid.ki = ki;
    pid.kd = kd;
    pid.dt = dt;
    pid.output_min = output_min;
    pid.output_max = output_max;
}

float PID_Update(float target, float measurement)
{
    float e_current = measurement - target;

    pid.integral = pid.integral + e_current * pid.dt;
    float diff = (e_current - pid.prev) / pid.dt;

    float u = pid.kp * e_current + pid.ki * pid.integral + pid.kd * diff;
    pid.prev = e_current;
    return u;
}

void PID_Reset(void)
{
}
