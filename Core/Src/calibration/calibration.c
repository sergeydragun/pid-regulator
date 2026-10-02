#include "calibration.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "cmsis_os2.h"

#include "pid_calculator.h"
#include "tim.h"
#include "hardware/encoder.h"
#include "hardware/motor.h"

#include "calibration_logger.h"
#include "static_compensation.h"


/* -------------------------------------------------------------------------- */
/* Target range                                                               */
/* -------------------------------------------------------------------------- */

#define CALIBRATION_TARGET_MIN             0
#define CALIBRATION_TARGET_MAX             40
#define CALIBRATION_TARGET_STEP            2


/* -------------------------------------------------------------------------- */
/* Control                                                                     */
/* -------------------------------------------------------------------------- */

#define CALIBRATION_PERIOD_MS              20U
#define CALIBRATION_DT                     0.020f

/*
 * Start with PD.
 *
 * The feedforward term is what will learn the static disturbance.
 */
#define CALIBRATION_KP                     6.0f
#define CALIBRATION_KI                     0.0f
#define CALIBRATION_KD                     0.0f

#define CALIBRATION_OUTPUT_MIN            (-100.0f)
#define CALIBRATION_OUTPUT_MAX             100.0f


/* -------------------------------------------------------------------------- */
/* Stability                                                                   */
/* -------------------------------------------------------------------------- */

#define CALIBRATION_STABLE_SAMPLES         10U

#define POSITION_TOLERANCE                 1
#define POSITION_DELTA_TOLERANCE           1


/*
 * After becoming stable, don't use one sample.
 *
 * 25 samples * 20 ms = 500 ms measurement window.
 */
#define CALIBRATION_AVERAGE_SAMPLES        25U


/* -------------------------------------------------------------------------- */
/* Learning                                                                    */
/* -------------------------------------------------------------------------- */

/*
 * 1.0:
 *   full correction on every iteration.
 *
 * 0.5:
 *   half correction.
 *
 * 0.25:
 *   very conservative.
 *
 * Start with 0.5.
 */
#define CALIBRATION_LEARNING_ALPHA          0.5f


/*
 * We consider the feedforward converged when the average
 * position error becomes smaller than this value.
 *
 * Although the encoder returns int32_t, averaging over many
 * samples gives us sub-count resolution when the mechanism
 * dithers around the target.
 */
#define CALIBRATION_MEAN_ERROR_TOLERANCE    0.20f


/*
 * Maximum number of feedforward learning iterations
 * for one position.
 */
#define CALIBRATION_MAX_LEARNING_ITERATIONS 12U


/* -------------------------------------------------------------------------- */
/* Timeout                                                                     */
/* -------------------------------------------------------------------------- */

#define CALIBRATION_TIMEOUT_MS              5000U


/* -------------------------------------------------------------------------- */
/* Helpers                                                                     */
/* -------------------------------------------------------------------------- */

static float clamp(
    float value,
    float min,
    float max
)
{
    if (value < min)
    {
        return min;
    }

    if (value > max)
    {
        return max;
    }

    return value;
}


static int32_t abs_i32(
    int32_t value
)
{
    return value < 0
        ? -value
        : value;
}


static uint32_t output_to_ccr(
    float output
)
{
    if (output < 0.0f)
    {
        output = -output;
    }

    if (output > 100.0f)
    {
        output = 100.0f;
    }


    const uint32_t arr =
        __HAL_TIM_GET_AUTORELOAD(&htim1);


    return (uint32_t)(
        output /
        100.0f *
        (float)(arr + 1U)
    );
}


/* -------------------------------------------------------------------------- */
/* Calibration                                                                 */
/* -------------------------------------------------------------------------- */

void Calibration_Run(void)
{
    if (!CalibrationLogger_Init())
    {
        Motor_SetOutput(0.0f);
        return;
    }


    StaticCompensation_Reset();


    const uint32_t tick_frequency =
        osKernelGetTickFreq();


    uint32_t period_ticks =
        (
            (
                (uint32_t)CALIBRATION_PERIOD_MS *
                tick_frequency
            )
            + 999U
        ) / 1000U;


    if (period_ticks == 0U)
    {
        period_ticks = 1U;
    }


    uint32_t timeout_ticks =
        (
            (
                (uint32_t)CALIBRATION_TIMEOUT_MS *
                tick_frequency
            )
        ) / 1000U;


    if (timeout_ticks == 0U)
    {
        timeout_ticks = 1U;
    }


    /*
     * PD controller.
     *
     * No integral action during learning.
     */
    PID_Init(
        CALIBRATION_KP,
        CALIBRATION_KI,
        CALIBRATION_KD,
        CALIBRATION_DT,
        CALIBRATION_OUTPUT_MIN,
        CALIBRATION_OUTPUT_MAX
    );


    Motor_SetOutput(0.0f);

    CalibrationLogger_Start();


    /*
     * The compensation function should be smooth.
     *
     * Therefore use the previous point's compensation
     * as the initial guess for the next point.
     *
     * Example:
     *
     * target 0 -> u_stat = 0
     * target 2 -> start near 0
     * target 4 -> start near compensation for 2
     * etc.
     */
    float previous_u_stat = 0.0f;


    /* ---------------------------------------------------------------------- */
    /* Target loop                                                            */
    /* ---------------------------------------------------------------------- */

    for (
        int32_t target = CALIBRATION_TARGET_MIN;
        target <= CALIBRATION_TARGET_MAX;
        target += CALIBRATION_TARGET_STEP
    )
    {
        /*
         * Initial estimate for this position.
         */
        float u_stat =
            previous_u_stat;


        bool target_completed = false;


        /* ------------------------------------------------------------------ */
        /* Iterative feedforward learning                                     */
        /* ------------------------------------------------------------------ */

        for (
            uint32_t learning_iteration = 0U;
            learning_iteration <
                CALIBRATION_MAX_LEARNING_ITERATIONS;
            learning_iteration++
        )
        {
            PID_Reset();


            uint32_t stable_samples = 0U;


            bool measuring = false;

            uint32_t average_samples = 0U;


            float sum_error = 0.0f;

            float sum_u_fb = 0.0f;

            int32_t sum_position = 0;


            int32_t previous_position =
                Encoder_GetPosition();


            const uint32_t start_time =
                osKernelGetTickCount();


            uint32_t next_wake =
                osKernelGetTickCount();


            for (;;)
            {
                /*
                 * ----------------------------------------------------------
                 * Fixed 50-Hz schedule
                 * ----------------------------------------------------------
                 */
                next_wake += period_ticks;


                /*
                 * ----------------------------------------------------------
                 * Measurement
                 * ----------------------------------------------------------
                 */
                const int32_t position =
                    Encoder_GetPosition();


                /*
                 * ----------------------------------------------------------
                 * FEEDBACK ONLY
                 *
                 * This is the amount by which the current u_stat
                 * is insufficient or excessive.
                 * ----------------------------------------------------------
                 */
                const float u_fb =
                    PID_Update(
                        (float)target,
                        (float)position
                    );


                /*
                 * ----------------------------------------------------------
                 * Total actuator command
                 * ----------------------------------------------------------
                 */
                const float u =
                    clamp(
                        u_stat + u_fb,
                        CALIBRATION_OUTPUT_MIN,
                        CALIBRATION_OUTPUT_MAX
                    );


                Motor_SetOutput(u);


                /*
                 * ----------------------------------------------------------
                 * Errors
                 * ----------------------------------------------------------
                 */
                const int32_t error =
                    target - position;


                const int32_t position_delta =
                    position - previous_position;


                const bool position_is_close =
                    abs_i32(error) <=
                    POSITION_TOLERANCE;


                const bool position_is_stable =
                    abs_i32(position_delta) <=
                    POSITION_DELTA_TOLERANCE;


                /*
                 * ----------------------------------------------------------
                 * Stability detection
                 * ----------------------------------------------------------
                 */
                if (!position_is_close ||
                    !position_is_stable)
                {
                    stable_samples = 0U;

                    measuring = false;

                    average_samples = 0U;

                    sum_error = 0.0f;

                    sum_u_fb = 0.0f;

                    sum_position = 0;
                }
                else
                {
                    stable_samples++;


                    /*
                     * Only start averaging after the system has
                     * been stable for ~200 ms.
                     */
                    if (!measuring &&
                        stable_samples >=
                            CALIBRATION_STABLE_SAMPLES)
                    {
                        measuring = true;

                        average_samples = 0U;

                        sum_error = 0.0f;

                        sum_u_fb = 0.0f;

                        sum_position = 0;
                    }


                    /*
                     * ------------------------------------------------------
                     * Measurement window
                     * ------------------------------------------------------
                     */
                    if (measuring)
                    {
                        sum_error +=
                            (float)error;


                        sum_u_fb +=
                            u_fb;


                        sum_position +=
                            position;


                        average_samples++;


                        /*
                         * 500-ms average window completed.
                         */
                        if (
                            average_samples >=
                                CALIBRATION_AVERAGE_SAMPLES
                        )
                        {
                            const float average_error =
                                sum_error /
                                (float)average_samples;


                            const float average_u_fb =
                                sum_u_fb /
                                (float)average_samples;


                            const int32_t average_position =
                                sum_position /
                                (int32_t)average_samples;


                            /*
                             * --------------------------------------------------
                             * CONVERGED
                             * --------------------------------------------------
                             */
                            if (
                                fabsf(average_error) <=
                                    CALIBRATION_MEAN_ERROR_TOLERANCE
                            )
                            {
                                /*
                                 * The feedforward value now represents
                                 * the required static compensation.
                                 */
                                StaticCompensation_Set(
                                    target,
                                    u_stat
                                );


                                previous_u_stat =
                                    u_stat;


                                /*
                                 * Exactly one result line for this target.
                                 */
                                const int32_t
                                    u_centi_percent =
                                    (int32_t)(
                                        u_stat *
                                        100.0f
                                    );


                                CalibrationLogger_Result(
                                    target,
                                    average_position,
                                    u_centi_percent
                                );


                                target_completed = true;

                                break;
                            }


                            /*
                             * --------------------------------------------------
                             * LEARNING UPDATE
                             * --------------------------------------------------
                             *
                             * This is the important equation:
                             *
                             * u_stat_new =
                             *     u_stat_old +
                             *     alpha * average_u_fb
                             *
                             * The feedback controller tells us directly
                             * how much static compensation is missing.
                             */
                            u_stat +=
                                CALIBRATION_LEARNING_ALPHA *
                                average_u_fb;


                            /*
                             * Safety limit.
                             */
                            u_stat =
                                clamp(
                                    u_stat,
                                    CALIBRATION_OUTPUT_MIN,
                                    CALIBRATION_OUTPUT_MAX
                                );


                            /*
                             * Stop this learning iteration.
                             *
                             * The NEXT iteration starts from exactly the
                             * current u_stat and lets the PD controller
                             * settle again.
                             */
                            break;
                        }
                    }
                }


                /*
                 * ----------------------------------------------------------
                 * Timeout
                 * ----------------------------------------------------------
                 */
                const uint32_t elapsed =
                    osKernelGetTickCount() -
                    start_time;


                if (elapsed >= timeout_ticks)
                {
                    const uint32_t duty =
                        output_to_ccr(u_stat);


                    const int32_t u_milli =
                        (int32_t)(
                            u_stat *
                            1000.0f
                        );


                    CalibrationLogger_Timeout(
                        target,
                        position,
                        u_milli,
                        duty
                    );


                    Motor_SetOutput(0.0f);

                    return;
                }


                previous_position = position;


                /*
                 * Next 20-ms control tick.
                 */
                (void)osDelayUntil(
                    next_wake
                );
            }


            if (target_completed)
            {
                break;
            }
        }


        /*
         * The target couldn't be learned within the allowed
         * number of iterations.
         */
        if (!target_completed)
        {
            Motor_SetOutput(0.0f);

            return;
        }
    }


    /* ---------------------------------------------------------------------- */
    /* Finished                                                                */
    /* ---------------------------------------------------------------------- */

    Motor_SetOutput(0.0f);

    CalibrationLogger_Done();
}