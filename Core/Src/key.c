#include "key.h"
#include "app.h"
#include "config.h"
#include "gpio.h"

static uint32_t g_key_tick = 0;
static uint8_t g_key_old[4] = {0};  // B1-B4的上一次状态

void KEY_Init(void)
{
    g_key_tick = 0;
}

void KEY_Process(void)
{
    if (uwTick - g_key_tick < KEY_SCAN_INTERVAL)
        return;
    
    g_key_tick = uwTick;
    
    /* 读取4个按键的当前状态 */
    uint8_t key_current[4];
    key_current[0] = !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);  // B1
    key_current[1] = !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);  // B2
    key_current[2] = !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);  // B3
    key_current[3] = !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);  // B4
    
    /* 边沿检测 */
    for (int i = 0; i < 4; i++) {
        if (key_current[i] && !g_key_old[i]) {
            /* 按键按下 */
            APP_HandleKeyInput(i + 1);
        }
        g_key_old[i] = key_current[i];
    }
}
