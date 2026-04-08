#include "pwm.h"
#include "main.h"
#include "config.h"

extern TIM_HandleTypeDef htim2;

void PWM_SetOutput(uint16_t freq, uint8_t duty)
{
    uint32_t period = 1000000 / freq - 1;       
    uint32_t pulse = (period + 1) * duty / 100;
    __HAL_TIM_SET_AUTORELOAD(&htim2, period);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);
}

