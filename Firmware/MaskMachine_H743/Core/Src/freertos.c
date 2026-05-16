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
#include "app_config.h"
#include "app_main.h"
#include "app_types.h"

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
osMessageQueueId_t appEventQueueHandle;
const osMessageQueueAttr_t appEventQueue_attributes = {
  .name = "appEventQueue"
};

osThreadId_t appControlTaskHandle;
const osThreadAttr_t appControlTask_attributes = {
  .name = "appControl",
  .stack_size = APP_CONTROL_TASK_STACK_WORDS * 4U,
  .priority = (osPriority_t) (osPriorityNormal + APP_CONTROL_TASK_PRIO),
};

osThreadId_t appUiTaskHandle;
const osThreadAttr_t appUiTask_attributes = {
  .name = "appUi",
  .stack_size = APP_UI_TASK_STACK_WORDS * 4U,
  .priority = (osPriority_t) (osPriorityNormal + APP_UI_TASK_PRIO),
};

osThreadId_t appDiagTaskHandle;
const osThreadAttr_t appDiagTask_attributes = {
  .name = "appDiag",
  .stack_size = APP_DIAG_TASK_STACK_WORDS * 4U,
  .priority = (osPriority_t) (osPriorityNormal + APP_DIAG_TASK_PRIO),
};

osThreadId_t appPollTaskHandle;
const osThreadAttr_t appPollTask_attributes = {
  .name = "appPoll",
  .stack_size = APP_POLL_TASK_STACK_WORDS * 4U,
  .priority = (osPriority_t) (osPriorityNormal + APP_POLL_TASK_PRIO),
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
  appEventQueueHandle = osMessageQueueNew(APP_EVENT_QUEUE_DEPTH, sizeof(app_event_t), &appEventQueue_attributes);
  App_Main_SetEventQueue(appEventQueueHandle);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  (void)App_Main_Init();
  appControlTaskHandle = osThreadNew(App_Task_Control, NULL, &appControlTask_attributes);
  appUiTaskHandle = osThreadNew(App_Task_Ui, NULL, &appUiTask_attributes);
  appDiagTaskHandle = osThreadNew(App_Task_Diag, NULL, &appDiagTask_attributes);
  appPollTaskHandle = osThreadNew(App_Task_InputPoll, NULL, &appPollTask_attributes);
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
  App_Task_Idle(argument);
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
