/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_12, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);

  /*Configure GPIO pins : PC13 PC14 PC15 PC8
                           PC9 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */
/* 按键读取函数 */
uint8_t key_old = 0;
uint8_t key_value = 0;
uint8_t key_down = 0;
uint8_t key_up = 0;

void KEY_Read(void)
{

    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == 0)      key_value = 1;
    else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == 0) key_value = 2;
    else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == 0) key_value = 3;
    else if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 0) key_value = 4;
    else key_value = 0;

    if(key_value != key_old) 
    {
        if(key_value != 0) key_down = key_value;
        else key_up = key_old;
    }
    else 
    {
        key_down = 0;
        key_up = 0;
    }

    key_old = key_value; 
}

/* LED显示函数 - LD1~LD8对应PC13~PC8和其他引脚 */
void LED_Disp(uint8_t ucLed)
{
    /*
     * 根据你的硬件：
     * LD1-LD8 连接到 GPIO_PIN_13~8 (PC13, PC14, PC15, PC8, PC9, PC10, PC11, PC12)
     * 
     * ucLed的每一位对应一个LED：
     * bit0 -> LD1 (PC13)
     * bit1 -> LD2 (PC14)
     * bit2 -> LD3 (PC15)
     * bit3 -> LD4 (PC8)
     * bit4 -> LD5 (PC9)
     * bit5 -> LD6 (PC10)
     * bit6 -> LD7 (PC11)
     * bit7 -> LD8 (PC12)
     */
    
    GPIOC->ODR = ~ucLed << 8;  // 取反并左移8位（因为从PC8开始）
    
    /* LED锁存 */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
}

/* LCD写函数 */
void LCD_Write(uint8_t RS, uint16_t Value)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
    GPIOC->ODR = Value;
    
    if(RS == 0)
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
    else
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
}

/* USER CODE END 2 */
