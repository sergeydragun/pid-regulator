//
// Created by sergey on 10/1/26.
//
#include "global_funcs.h"
#include "global_vars.h"
#include "control_layer/adrc.h"
#include "application_layer/position_controller.h"
#include "hardware_layer/motor.h"
#include "cmsis_os.h"
#include "tim.h"

// Параметры объекта, снятые разомкнутым тестом (импульсы ±55..80%):
//  - положительный выход мотора УМЕНЬШАЕТ показания энкодера;
//  - вал не сдвигается, пока |выход| < ~55% (пружина + сухое трение);
//  - выше порога разгон ~4000..7000 отсчётов/с² на 1% сверх порога
//    (энкодер в режиме TI12, ход между упорами ~186 отсчётов).
#define MOTOR_DIRECTION                  (-1.0f)
#define MOTOR_OUTPUT_LIMIT                 83.0f

// Компенсация мёртвой зоны: к любому ненулевому u добавляется ±FRICTION_COMP.
// Внутри ±FRICTION_RAMP компенсация линейно уходит в 0, чтобы не дёргать
// мотор между +50% и -50% от шума вокруг нуля.
#define FRICTION_COMP                      45.0f
#define FRICTION_RAMP                       2.0f

#define ADRC_DT                  (POSITION_CONTROLLER_PERIOD_MS / 1000.0f)
// Подбор на стенде (шаг ±50 отсчётов):
//  - WO: 45 -> задержка старта ~350 мс (ESO медленно перестраивает x3 при смене
//    направления трения/пружины); 100 -> ~70 мс; 160 -> шум квантования энкодера
//    раскачивает u на ±10%. WO*DT = 0.2, дискретный ESO далеко от границы 0.5..0.6.
//  - WC = WO/5.
//  - B0 намеренно чуть меньше реального b: при B0 = 1600 и тех же WC/WO контур
//    терял устойчивость, если b объекта вырастал вдвое (проверено B0 = 800).
//    С B0 = 3200 работает при b объекта от x0.5 до x2.
#define ADRC_WC                            20.0f   // полоса регулятора, рад/с
#define ADRC_WO                           100.0f   // полоса наблюдателя, рад/с
#define ADRC_B0                          3200.0f   // отсчёты/с² на 1% выхода

// Хоминг: счётчик энкодера стартует с 0 там, где вал оказался при сбросе,
// поэтому координаты "плавают" между запусками. Прижимаем вал к нижнему упору
// (положительный выход мотора) и присваиваем упору фиксированную координату.
// Ход ~186 отсчётов, точка покоя пружины ~60..90 над нижним упором,
// так что при -80 ноль оказывается примерно в точке покоя.
#define HOMING_OUTPUT                      70.0f
#define HOMING_PUSH_MS                     500U
#define HOMING_SETTLE_MS                   500U
#define HOMING_LOWER_STOP_POS             (-80)

static void PositionController_Home(void)
{
    Motor_SetOutput(HOMING_OUTPUT);
    osDelay(HOMING_PUSH_MS);

    __HAL_TIM_SET_COUNTER(&htim2, (uint32_t)(int32_t)HOMING_LOWER_STOP_POS);

    Motor_SetOutput(0.0f);
    osDelay(HOMING_SETTLE_MS);
}

void PositionController_Init(void)
{
    Motor_Init();
    PositionController_Home();

    const int32_t initial_pos = (int32_t)__HAL_TIM_GET_COUNTER(&htim2);

    const float kp = ADRC_WC * ADRC_WC;
    const float kd = 2.0f * ADRC_WC;

    ADRC_Init(&g_adrc, ADRC_WO, ADRC_B0, kp, kd, ADRC_DT,
              MOTOR_OUTPUT_LIMIT - FRICTION_COMP, initial_pos);
}

float PositionController_Update(float target, int32_t position)
{
    const float u = ADRC_Update(&g_adrc, target, position);

    const float comp = FRICTION_COMP * clamp(u / FRICTION_RAMP, -1.0f, 1.0f);

    return MOTOR_DIRECTION * clamp(
        u + comp,
        -MOTOR_OUTPUT_LIMIT,
        MOTOR_OUTPUT_LIMIT
    );
}
