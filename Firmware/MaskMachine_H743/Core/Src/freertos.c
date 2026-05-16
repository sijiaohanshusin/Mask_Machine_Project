/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mm_config.h"
#include "mm_runtime.h"
#include "mm_types.h"

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
osMessageQueueId_t mmEventQueueHandle;
const osMessageQueueAttr_t mmEventQueue_attributes = {
  .name = "mmEventQueue"
};

osThreadId_t mmControlTaskHandle;
const osThreadAttr_t mmControlTask_attributes = {
  .name = "mmControl",
  .stack_size = MM_CONTROL_TASK_STACK_WORDS * 4U,
  .priority = (osPriority_t) (osPriorityNormal + MM_CONTROL_TASK_PRIO),
};

osThreadId_t mmUiTaskHandle;
const osThreadAttr_t mmUiTask_attributes = {
  .name = "mmUi",
  .stack_size = MM_UI_TASK_STACK_WORDS * 4U,
  .priority = (osPriority_t) (osPriorityNormal + MM_UI_TASK_PRIO),
};

osThreadId_t mmDiagTaskHandle;
const osThreadAttr_t mmDiagTask_attributes = {
  .name = "mmDiag",
  .stack_size = MM_DIAG_TASK_STACK_WORDS * 4U,
  .priority = (osPriority_t) (osPriorityNormal + MM_DIAG_TASK_PRIO),
};

osThreadId_t mmPollTaskHandle;
const osThreadAttr_t mmPollTask_attributes = {
  .name = "mmPoll",
  .stack_size = MM_POLL_TASK_STACK_WORDS * 4U,
  .priority = (osPriority_t) (osPriorityNormal + MM_POLL_TASK_PRIO),
};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

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
  mmEventQueueHandle = osMessageQueueNew(MM_EVENT_QUEUE_DEPTH, sizeof(mm_event_t), &mmEventQueue_attributes);
  Mm_Runtime_SetEventQueue(mmEventQueueHandle);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  (void)Mm_Runtime_Init();
  mmControlTaskHandle = osThreadNew(Mm_Task_Control, NULL, &mmControlTask_attributes);
  mmUiTaskHandle = osThreadNew(Mm_Task_Ui, NULL, &mmUiTask_attributes);
  mmDiagTaskHandle = osThreadNew(Mm_Task_Diag, NULL, &mmDiagTask_attributes);
  mmPollTaskHandle = osThreadNew(Mm_Task_InputPoll, NULL, &mmPollTask_attributes);
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
  Mm_Task_Idle(argument);
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

