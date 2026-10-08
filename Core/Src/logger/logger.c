#include "logger.h"
#include "queue.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

QueueHandle_t xLogQueue = NULL;
TaskHandle_t xLoggerTaskHandle = NULL;
volatile uint8_t uart_dma_busy = 0;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        uart_dma_busy = 0;

        vTaskNotifyGiveFromISR(xLoggerTaskHandle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void vLoggerTask(void *pvParameters)
{
    LogMessage_t msg;

    for (;;)
    {
        if (xQueueReceive(xLogQueue, &msg, portMAX_DELAY) == pdTRUE)
        {
            uint16_t len = strlen(msg.text);

            if (len == 0) continue;

            uart_dma_busy = 1;

            HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(&huart2, (uint8_t*)msg.text, len);
            if (status == HAL_OK)
            {
                if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(500)) == 0)
                {
                    uart_dma_busy = 0;
                    HAL_UART_DMAStop(&huart2);
                }
            }
            else
            {
                uart_dma_busy = 0;
            }
        }
    }
}

void Log_Write(const char *format, ...)
{
    if (xLogQueue == NULL) return;

    LogMessage_t msg;
    va_list args;

    va_start(args, format);
    vsnprintf(msg.text, LOG_BUFFER_SIZE, format, args);
    va_end(args);

    uint16_t len = strlen(msg.text);

    if (len > 0 && len < LOG_BUFFER_SIZE - 2 && msg.text[len - 1] != '\n')
    {
        strcat(msg.text, "\r\n");
    }

    xQueueSend(xLogQueue, &msg, pdMS_TO_TICKS(20));
}

void Logger_Init(void)
{
    xLogQueue = xQueueCreate(LOG_QUEUE_SIZE, sizeof(LogMessage_t));

    if (xLogQueue != NULL)
    {
        xTaskCreate(vLoggerTask, "Logger", 256, NULL, 2, &xLoggerTaskHandle);
    }
}