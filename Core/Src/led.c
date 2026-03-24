#include "led.h"
#include "gpio.h"

void LED_Init(void)
{
    /* 所有LED初始化为熄灭 */
    for (int i = 0; i < 8; i++) {
        LED_TurnOff(i);
    }
}

void LED_TurnOn(uint8_t led_id)
{
    /* 将对应位设置为1 */
    uint8_t led_value = (1 << led_id);
    LED_Disp(led_value);  // 调用gpio.c的HAL层函数
}

void LED_TurnOff(uint8_t led_id)
{
    /* 将对应位设置为0 */
    uint8_t led_value = ~(1 << led_id);
    LED_Disp(led_value);
}

void LED_SetLD1OnTime(uint32_t duration)
{
    /* 这个函数在app.c中调用，传递时长给app.c管理 */
    LED_TurnOn(LED1);
}

void LED_SetLD2AlarmTime(uint32_t duration)
{
    LED_TurnOn(LED2);
}