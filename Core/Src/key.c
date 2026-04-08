#include "key.h"
#include "main.h" // For global variables
#include "config.h"
#include "gpio.h"
#include <string.h>

uint8_t key_down = 0;
uint8_t key_up = 0;
uint8_t key_value = 0;
uint8_t key_old = 0; // 上一次扫描的key_value

// 按键扫描，修复原bug（B1和B2引脚冲突）
void key_read(void)
{
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == 0)      key_value = 1;
    else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == 0) key_value = 2;
    else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == 0) key_value = 3;
    else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 0) key_value = 4;
    else key_value = 0;

    key_down = key_value & (key_value ^ key_old);    // 按下事件
    key_up   = ~key_value & (key_value ^ key_old);   // 松开事件
    key_old  = key_value;   // 保存本次状态
}

// 供主循环调用的按键处理任务
