#include "led.h"
#include "gpio.h"
#include "config.h"
#include "main.h"

// 记录LED当前的点亮挂载状态（每一位对应一个灯）
static uint8_t g_led_state = 0x00;

// LED初始化，全灭
void LED_Init(void)
{
    g_led_state = 0x00;
    LED_Disp(g_led_state); 
}

// 点亮某一个特定的 LED
void LED_TurnOn(uint8_t led_id)
{
    g_led_state |= (1 << led_id);
    LED_Disp(g_led_state);        
}

// 熄灭某一个特定的 LED
void LED_TurnOff(uint8_t led_id)
{
    g_led_state &= ~(1 << led_id);
    LED_Disp(g_led_state);
}