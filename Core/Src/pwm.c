#include "pwm.h"
#include "gpio.h"
#include "config.h"


void PWM_Init(void)
{
    /* PA1配置为输出（初始值1KHz, 50% duty） */
    PWM_SetOutput(NORMAL_FREQ, NORMAL_DUTY);
}

void PWM_SetOutput(uint16_t freq, uint8_t duty)
{
    /* 
     * 使用定时器TIM2输出PWM到PA1
     * 这部分需要根据具体的硬件定时器配置
     * 
     * 伪代码示例（实际需要配置TIM2）：
     * 
     * uint32_t period = 1000000 / freq;    // 周期(us)
     * uint32_t pulse = period * duty / 100; // 脉宽(us)
     * 
     * TIM_SetPeriod(period);
     * TIM_SetPulse(pulse);
     * TIM_Start();
     */
}
