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
#include "main.h"
#include "config.h"
#include "led.h"
#include "pwm.h"
#include "lcd.h"
#include "key.h"
#include <string.h>
#include <stdio.h>

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
#include "main.h"
#include "config.h"
#include "led.h"
#include "pwm.h"
#include "lcd.h"
#include "key.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "config.h"
#include "led.h"
#include "pwm.h"
#include "lcd.h"
#include "key.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>


// main.c里的全局变量拿过来引用
extern uint8_t system_state;
extern char current_pwd[4];
extern char input_pwd[4];
extern uint32_t led1_on_time;
extern uint32_t led2_alarm_time;
extern uint32_t pwm_verify_time;
extern uint8_t error_count;
extern TIM_HandleTypeDef htim2;

// --- 模块核心状态与处理函数集中放置于此处 ---

void KEY_Process(void)
{
    static uint32_t key_tick = 0;
    if (uwTick - key_tick < 20) return; // 20ms消抖
    key_tick = uwTick;
    
    key_read(); // 发起硬件读取采集事件

    if (system_state == 0) {
        if (key_down >= 1 && key_down <= 3) {
            uint8_t digit = input_pwd[key_down - 1];
            if (digit == '@') digit = '0';
            else digit = '0' + ((digit - '0' + 1) % 10);
            input_pwd[key_down - 1] = digit;
        }
        else if (key_down == 4) {
            if (strncmp(input_pwd, current_pwd, 3) == 0) {
                // 如果密码正确，切换到配置2KHz高频状态并点亮LED1长亮五秒
                system_state = 1;
                led1_on_time = uwTick;
                pwm_verify_time = uwTick;
                error_count = 0;
            } else {
                // 如果密码错误，计错器加一；输错满三次则触发LED2警报
                strcpy(input_pwd, "@@@");
                error_count++;
                if (error_count >= 3) {
                    led2_alarm_time = uwTick; 
                    error_count = 0;
                }
            }
        }
    }
}

void LED_Process(void)
{
    if (led1_on_time > 0) {
        if (uwTick - led1_on_time < LED1_ON_DURATION) LED_TurnOn(LED1);
        else { LED_TurnOff(LED1); led1_on_time = 0; }
    }

    if (led2_alarm_time > 0) {
        if (uwTick - led2_alarm_time < LED2_ALARM_DURATION) {
            if (((uwTick - led2_alarm_time) / LED2_ALARM_INTERVAL) % 2 == 0)
                LED_TurnOn(LED2);
            else
                LED_TurnOff(LED2);
        } else {
            LED_TurnOff(LED2);
            led2_alarm_time = 0;
        }
    }
}

void PWM_Process(void)
{
    static uint8_t is_high_freq = 0; 
    if (pwm_verify_time > 0) {
        if (uwTick - pwm_verify_time < VERIFY_DURATION) {
            if (!is_high_freq) {
                PWM_SetOutput(VERIFY_FREQ, VERIFY_DUTY);
                is_high_freq = 1;
            }
        } else {
            PWM_SetOutput(NORMAL_FREQ, NORMAL_DUTY);
            is_high_freq = 0;
            pwm_verify_time = 0;
            system_state = 0; 
        }
    } else {
        if (is_high_freq) {
            PWM_SetOutput(NORMAL_FREQ, NORMAL_DUTY);
            is_high_freq = 0;
        }
    }
}

void LCD_Process(void)
{
    static uint32_t lcd_tick = 0;
    if (uwTick - lcd_tick < LCD_REFRESH_INTERVAL) return;
    lcd_tick = uwTick;
    
    char buf[20];
    
    if (system_state == 0) {
        LCD_DisplayStringLine(LCD_LINE_TITLE, (uint8_t *)"      PSD        ");
        sprintf(buf, "   %c %c %c         ", input_pwd[0], input_pwd[1], input_pwd[2]);
        LCD_DisplayStringLine(LCD_LINE_PSW_1, (uint8_t *)buf);
        LCD_DisplayStringLine(LCD_LINE_DUTY, (uint8_t *)"                 "); 
    }
    else if (system_state == 1) {
        LCD_DisplayStringLine(LCD_LINE_TITLE, (uint8_t *)"      STA        ");
        uint32_t f = (pwm_verify_time > 0) ? VERIFY_FREQ : NORMAL_FREQ;
        uint32_t d = (pwm_verify_time > 0) ? VERIFY_DUTY : NORMAL_DUTY;
        
        sprintf(buf, "   F:%uHz         ", (unsigned int)f);
        LCD_DisplayStringLine(LCD_LINE_FREQ, (uint8_t *)buf);
        sprintf(buf, "   D:%u%%          ", (unsigned int)d);
        LCD_DisplayStringLine(LCD_LINE_DUTY, (uint8_t *)buf);
    }
}

/* LED显示函数 - LD1~LD8对应PC13~PC8和其他引脚 */
void LED_Disp(uint8_t ucLed)
{
    GPIOC->ODR = ~ucLed << 8;  
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

