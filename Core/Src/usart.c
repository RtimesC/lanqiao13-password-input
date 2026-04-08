/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PC4     ------> USART1_TX
    PC5     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PC4     ------> USART1_TX
    PC5     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_4|GPIO_PIN_5);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* UART接收缓冲 */
#define UART_RX_BUF_SIZE 20
static uint8_t g_uart_rx_buf[UART_RX_BUF_SIZE];
static uint8_t g_uart_rx_index = 0;
static uint8_t g_uart_rx_data = 0;

/* 初始化UART并启动接收中断 */
void UART_Init(void)
{
    g_uart_rx_index = 0;
    
    /* 启动UART接收中断（接收1个字节后触发中断） */
    HAL_UART_Receive_IT(&huart1, &g_uart_rx_data, 1);
}

/* 发送单个字符 */
void UART_SendChar(uint8_t ch)
{
    HAL_UART_Transmit(&huart1, &ch, 1, 100);
}

/* 发送字符串 */
void UART_SendString(char *str)
{
    while (*str) {
        UART_SendChar(*str);
        str++;
    }
}

/* UART接收中断回调函数 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        /* 将接收到的字符存入缓冲 */
        g_uart_rx_buf[g_uart_rx_index++] = g_uart_rx_data;
        
        /* 缓冲满或接收到结束符('\r') */
        if (g_uart_rx_index >= UART_RX_BUF_SIZE - 1 || 
            g_uart_rx_data == '\r') {
            
            g_uart_rx_buf[g_uart_rx_index] = '\0';
            
            /* 处理接收到的命令 */
            UART_HandleCmd((char *)g_uart_rx_buf);
            
            /* 重置缓冲 */
            g_uart_rx_index = 0;
        }
        
        /* 继续接收下一个字符 */
        HAL_UART_Receive_IT(&huart1, &g_uart_rx_data, 1);
    }
}

/* UART处理函数（可选，用于超时检测） */
void UART_Process(void)
{
    /* 如果有接收缓冲数据，但超过1秒没有新数据，则重置 */
    static uint32_t last_rx_time = 0;
    
    if (g_uart_rx_index > 0) {
        if (uwTick - last_rx_time > 1000) {
            /* 接收超时，清空缓冲 */
            g_uart_rx_index = 0;
        }
    } else {
        last_rx_time = uwTick;
    }
}

/* USER CODE END 1 */

#include "main.h"
#include <string.h>

void UART_HandleCmd(char *cmd) {
    char old_pwd[4], new_pwd[4];
    if (strlen(cmd) >= 7 && cmd[3] == '-') {
        strncpy(old_pwd, cmd, 3); old_pwd[3] = '\0';
        strncpy(new_pwd, cmd + 4, 3); new_pwd[3] = '\0';
        if (strcmp(old_pwd, current_pwd) == 0) {
            strcpy(current_pwd, new_pwd);
            UART_SendString("Password changed!\r\n");
        } else {
            UART_SendString("Wrong password!\r\n");
        }
    }
}
#include "main.h"
#include <string.h>

void UART_HandleCmd(char *cmd) {
    char old_pwd[4], new_pwd[4];
    if (strlen(cmd) >= 7 && cmd[3] == '-') {
        strncpy(old_pwd, cmd, 3); old_pwd[3] = '\0';
        strncpy(new_pwd, cmd + 4, 3); new_pwd[3] = '\0';
        if (strcmp(old_pwd, current_pwd) == 0) {
            strcpy(current_pwd, new_pwd);
            UART_SendString("Password changed!\r\n");
        } else {
            UART_SendString("Wrong password!\r\n");
        }
    }
}
