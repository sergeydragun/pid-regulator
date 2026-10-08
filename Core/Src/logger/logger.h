//
// Created by sergey on 9/29/26.
//

#ifndef PID_REGULATOR_CALIBRATION_LOGGER_H
#define PID_REGULATOR_CALIBRATION_LOGGER_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef huart2;

#define LOG_QUEUE_SIZE 15
#define LOG_BUFFER_SIZE 128

typedef struct
{
    char text[LOG_BUFFER_SIZE];
} LogMessage_t;

extern QueueHandle_t xLogQueue;
extern TaskHandle_t xLoggerTaskHandle;

void Log_Write(const char *format, ...);
void vLoggerTask(void *pvParameters);
void Logger_Init(void);

extern volatile uint8_t uart_dma_busy;

#endif
