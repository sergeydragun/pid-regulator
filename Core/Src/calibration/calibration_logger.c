#include "calibration_logger.h"

#include <stdio.h>
#include <inttypes.h>

#include "cmsis_os2.h"


/* -------------------------------------------------------------------------- */
/* Configuration                                                              */
/* -------------------------------------------------------------------------- */

#define CALIBRATION_LOG_QUEUE_LENGTH        32U
#define CALIBRATION_LOGGER_STACK_SIZE       1024U

/*
 * Logger deliberately has lower priority than the calibration task.
 *
 * So even a slow/blocking UART cannot preempt the PID loop.
 */
#define CALIBRATION_LOGGER_PRIORITY          osPriorityLow


/* -------------------------------------------------------------------------- */
/* Types                                                                      */
/* -------------------------------------------------------------------------- */

typedef enum
{
    CAL_LOG_START = 0,
    CAL_LOG_RESULT,
    CAL_LOG_TIMEOUT,
    CAL_LOG_DONE
} CalibrationLogType;


typedef struct
{
    CalibrationLogType type;

    int32_t target;
    int32_t position;

    int32_t u_milli;
    uint32_t duty;

    int32_t average_position;
    int32_t u_centi_percent;

} CalibrationLogMessage;


/* -------------------------------------------------------------------------- */
/* Static state                                                               */
/* -------------------------------------------------------------------------- */

static osMessageQueueId_t s_log_queue = NULL;

static osThreadId_t s_logger_thread = NULL;

static volatile uint32_t s_dropped_messages = 0U;


/* -------------------------------------------------------------------------- */
/* Logger task                                                                */
/* -------------------------------------------------------------------------- */

static void CalibrationLogger_Task(void *argument)
{
    (void)argument;

    CalibrationLogMessage message;


    for (;;)
    {
        const osStatus_t status =
            osMessageQueueGet(
                s_log_queue,
                &message,
                NULL,
                osWaitForever
            );


        if (status != osOK)
        {
            continue;
        }


        /*
         * Inform the user if any log message was lost because
         * the queue was full.
         */
        if (s_dropped_messages != 0U)
        {
            const uint32_t dropped =
                s_dropped_messages;

            s_dropped_messages = 0U;

            printf(
                "ERROR,LOG_QUEUE_OVERFLOW,%" PRIu32 "\r\n",
                dropped
            );

            fflush(stdout);
        }


        switch (message.type)
        {
            case CAL_LOG_START:
            {
                printf(
                    "CALIBRATION_START\r\n"
                );

                break;
            }


            case CAL_LOG_RESULT:
            {
                /*
                 * Exactly ONE result line per target:
                 *
                 * CAL,target,average_position,u_centi_percent
                 *
                 * Example:
                 *
                 * CAL,12,12,845
                 *
                 * 845 -> 8.45 %
                 */
                printf(
                    "CAL,%"
                    PRId32
                    ",%"
                    PRId32
                    ",%"
                    PRId32
                    "\r\n",

                    message.target,
                    message.average_position,
                    message.u_centi_percent
                );

                break;
            }


            case CAL_LOG_TIMEOUT:
            {
                /*
                 * Timeout is also exactly one line for this target.
                 *
                 * Extra u and duty values make debugging much easier.
                 */
                printf(
                    "ERROR,TIMEOUT,%"
                    PRId32
                    ",%"
                    PRId32
                    ",u=%"
                    PRId32
                    ",duty=%"
                    PRIu32
                    "\r\n",

                    message.target,
                    message.position,
                    message.u_milli,
                    message.duty
                );

                break;
            }


            case CAL_LOG_DONE:
            {
                printf(
                    "CALIBRATION_DONE\r\n"
                );

                break;
            }


            default:
            {
                printf(
                    "ERROR,LOGGER_UNKNOWN_MESSAGE\r\n"
                );

                break;
            }
        }


        /*
         * Push stdout contents immediately to the UART backend.
         *
         * This is done in the logger task, never in the PID task.
         */
        fflush(stdout);
    }
}


/* -------------------------------------------------------------------------- */
/* Initialization                                                             */
/* -------------------------------------------------------------------------- */

bool CalibrationLogger_Init(void)
{
    /*
     * Already initialized.
     */
    if ((s_log_queue != NULL) &&
        (s_logger_thread != NULL))
    {
        return true;
    }


    s_log_queue =
        osMessageQueueNew(
            CALIBRATION_LOG_QUEUE_LENGTH,
            sizeof(CalibrationLogMessage),
            NULL
        );


    if (s_log_queue == NULL)
    {
        return false;
    }


    const osThreadAttr_t attributes =
    {
        .name = "CalLogger",
        .stack_size = CALIBRATION_LOGGER_STACK_SIZE,
        .priority = CALIBRATION_LOGGER_PRIORITY
    };


    s_logger_thread =
        osThreadNew(
            CalibrationLogger_Task,
            NULL,
            &attributes
        );


    if (s_logger_thread == NULL)
    {
        osMessageQueueDelete(s_log_queue);

        s_log_queue = NULL;

        return false;
    }


    return true;
}


/* -------------------------------------------------------------------------- */
/* Public logging functions                                                   */
/* -------------------------------------------------------------------------- */

static bool CalibrationLogger_Put(
    const CalibrationLogMessage *message
)
{
    if (s_log_queue == NULL)
    {
        return false;
    }


    const osStatus_t status =
        osMessageQueuePut(
            s_log_queue,
            message,
            0U,
            0U
        );


    if (status != osOK)
    {
        /*
         * NEVER wait for UART from the calibration task.
         */
        s_dropped_messages++;

        return false;
    }


    return true;
}


void CalibrationLogger_Start(void)
{
    const CalibrationLogMessage message =
    {
        .type = CAL_LOG_START
    };


    (void)CalibrationLogger_Put(&message);
}


void CalibrationLogger_Result(
    int32_t target,
    int32_t average_position,
    int32_t u_centi_percent
)
{
    const CalibrationLogMessage message =
    {
        .type = CAL_LOG_RESULT,

        .target = target,

        .average_position =
            average_position,

        .u_centi_percent =
            u_centi_percent
    };


    (void)CalibrationLogger_Put(&message);
}


void CalibrationLogger_Timeout(
    int32_t target,
    int32_t position,
    int32_t u_milli,
    uint32_t duty
)
{
    const CalibrationLogMessage message =
    {
        .type = CAL_LOG_TIMEOUT,

        .target = target,

        .position = position,

        .u_milli = u_milli,

        .duty = duty
    };


    (void)CalibrationLogger_Put(&message);
}


void CalibrationLogger_Done(void)
{
    const CalibrationLogMessage message =
    {
        .type = CAL_LOG_DONE
    };


    (void)CalibrationLogger_Put(&message);
}