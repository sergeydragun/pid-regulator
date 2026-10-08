/* USER CODE BEGIN Header */
/**

---

* File Name          : freertos.c
* Description        : Code for freertos applications

---

* @attention
*
* Copyright (c) 2026 STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*

---

*/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

#include "global_vars.h"
#include "tim.h"
#include "hardware_layer/motor.h"
#include "logger/logger.h"
#include "application_layer/position_controller.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

osThreadId_t pidTargetHandle;
const osThreadAttr_t pidTarget_attributes = {
.name = "pidTarget",
.stack_size = 128 * 4,
.priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t pidTaskHandle;
const osThreadAttr_t pidTask_attributes = {
.name = "pidTask",
.stack_size = 1024,
.priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
.name = "defaultTask",
.stack_size = 1024,
.priority = (osPriority_t) osPriorityNormal,
};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartPIDTarget(void *argument);
void StartPIDTask(void *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  pidTargetHandle = osThreadNew(
  StartPIDTarget,
  NULL,
  &pidTarget_attributes
  );

  pidTaskHandle = osThreadNew(
  StartPIDTask,
  NULL,
  &pidTask_attributes
  );

  Logger_Init();

  if (pidTargetHandle == NULL || pidTaskHandle == NULL)
  {
  Error_Handler();
  }
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
  /**
* @brief  Function implementing the defaultTask thread.
* @param  argument: Not used
* @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void StartPIDTarget(void *argument)
{
  for (;;)
  {
    g_target = (g_target == (int8_t)25)
                   ? (int8_t)-25
                   : (int8_t)25;

    osDelay(3000);
  }

}

void StartPIDTask(void *argument)
{
  // Initialize ADRC.

  PositionController_Init((int32_t)__HAL_TIM_GET_COUNTER(&htim2));

  for (;;)
  {
    float u = PositionController_Update(
        g_target,
        (int32_t)__HAL_TIM_GET_COUNTER(&htim2)
    );

    Motor_SetOutput(u);

    osDelay(10);
  }

}

/* USER CODE END Application */

