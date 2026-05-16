/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "main.h"
#include "stm32h7xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile mm_fault_record_t g_mm_fault_record;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void Fault_Blink(void);
void Mm_Fault_HandlerC(uint32_t *stack, uint32_t exc_return, uint32_t fault_id);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Fault_Record(uint32_t *stack, uint32_t exc_return, uint32_t fault_id)
{
  g_mm_fault_record.magic = MM_FAULT_MAGIC;
  g_mm_fault_record.fault_id = fault_id;
  g_mm_fault_record.exc_return = exc_return;

  if (stack != NULL)
  {
    g_mm_fault_record.r0 = stack[0];
    g_mm_fault_record.r1 = stack[1];
    g_mm_fault_record.r2 = stack[2];
    g_mm_fault_record.r3 = stack[3];
    g_mm_fault_record.r12 = stack[4];
    g_mm_fault_record.lr = stack[5];
    g_mm_fault_record.pc = stack[6];
    g_mm_fault_record.xpsr = stack[7];
  }
  else
  {
    g_mm_fault_record.r0 = 0u;
    g_mm_fault_record.r1 = 0u;
    g_mm_fault_record.r2 = 0u;
    g_mm_fault_record.r3 = 0u;
    g_mm_fault_record.r12 = 0u;
    g_mm_fault_record.lr = 0u;
    g_mm_fault_record.pc = 0u;
    g_mm_fault_record.xpsr = 0u;
  }

  g_mm_fault_record.cfsr = SCB->CFSR;
  g_mm_fault_record.hfsr = SCB->HFSR;
  g_mm_fault_record.dfsr = SCB->DFSR;
  g_mm_fault_record.afsr = SCB->AFSR;
  g_mm_fault_record.mmfar = SCB->MMFAR;
  g_mm_fault_record.bfar = SCB->BFAR;
  g_mm_fault_record.icsr = SCB->ICSR;
  g_mm_fault_record.shcsr = SCB->SHCSR;
  g_mm_fault_record.control = __get_CONTROL();
  g_mm_fault_record.msp = __get_MSP();
  g_mm_fault_record.psp = __get_PSP();
  g_mm_fault_record.primask = __get_PRIMASK();
  g_mm_fault_record.basepri = __get_BASEPRI();
}

void Mm_Fault_HandlerC(uint32_t *stack, uint32_t exc_return, uint32_t fault_id)
{
  __disable_irq();
  Fault_Record(stack, exc_return, fault_id);
  __DSB();
  __ISB();
  Fault_Blink();
}

static void Fault_Blink(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  volatile uint32_t delay;

  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  for (;;)
  {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
    for (delay = 0; delay < 900000U; delay++)
    {
      __NOP();
    }
  }
}

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim6;

/* USER CODE BEGIN EV */
extern LTDC_HandleTypeDef g_ltdc_handle;
extern DMA2D_HandleTypeDef g_dma2d_handle;

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */
  Mm_Fault_HandlerC(0, 0u, MM_FAULT_NMI);

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
__asm void HardFault_Handler(void)
{
  IMPORT Mm_Fault_HandlerC
  TST LR, #4
  ITE EQ
  MRSEQ R0, MSP
  MRSNE R0, PSP
  MOV R1, LR
  MOVS R2, #2
  B Mm_Fault_HandlerC
}
#if 0
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  Fault_Blink();

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}
#endif

/**
  * @brief This function handles Memory management fault.
  */
__asm void MemManage_Handler(void)
{
  IMPORT Mm_Fault_HandlerC
  TST LR, #4
  ITE EQ
  MRSEQ R0, MSP
  MRSNE R0, PSP
  MOV R1, LR
  MOVS R2, #3
  B Mm_Fault_HandlerC
}
#if 0
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */
  Fault_Blink();

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}
#endif

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
__asm void BusFault_Handler(void)
{
  IMPORT Mm_Fault_HandlerC
  TST LR, #4
  ITE EQ
  MRSEQ R0, MSP
  MRSNE R0, PSP
  MOV R1, LR
  MOVS R2, #4
  B Mm_Fault_HandlerC
}
#if 0
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */
  Fault_Blink();

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}
#endif

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
__asm void UsageFault_Handler(void)
{
  IMPORT Mm_Fault_HandlerC
  TST LR, #4
  ITE EQ
  MRSEQ R0, MSP
  MRSNE R0, PSP
  MOV R1, LR
  MOVS R2, #5
  B Mm_Fault_HandlerC
}
#if 0
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */
  Fault_Blink();

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}
#endif

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM6 global interrupt, DAC1_CH1 and DAC1_CH2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */

  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief This function handles LTDC global interrupt.
  */
void LTDC_IRQHandler(void)
{
  /* USER CODE BEGIN LTDC_IRQn 0 */

  /* USER CODE END LTDC_IRQn 0 */
  HAL_LTDC_IRQHandler(&g_ltdc_handle);
  /* USER CODE BEGIN LTDC_IRQn 1 */

  /* USER CODE END LTDC_IRQn 1 */
}

/**
  * @brief This function handles LTDC global error interrupt.
  */
void LTDC_ER_IRQHandler(void)
{
  /* USER CODE BEGIN LTDC_ER_IRQn 0 */

  /* USER CODE END LTDC_ER_IRQn 0 */
  HAL_LTDC_IRQHandler(&g_ltdc_handle);
  /* USER CODE BEGIN LTDC_ER_IRQn 1 */

  /* USER CODE END LTDC_ER_IRQn 1 */
}

/**
  * @brief This function handles DMA2D global interrupt.
  */
void DMA2D_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2D_IRQn 0 */

  /* USER CODE END DMA2D_IRQn 0 */
  HAL_DMA2D_IRQHandler(&g_dma2d_handle);
  /* USER CODE BEGIN DMA2D_IRQn 1 */

  /* USER CODE END DMA2D_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  (void)pcTaskName;
  Mm_Fault_HandlerC(0, 0u, MM_FAULT_STACK);
}

void vApplicationMallocFailedHook(void)
{
  Mm_Fault_HandlerC(0, 0u, MM_FAULT_MALLOC);
}

/* USER CODE END 1 */
