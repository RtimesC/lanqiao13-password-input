#ifndef __PWM_H
#define __PWM_H
#include "stm32g4xx_hal.h"
#include <stdint.h>
void PWM_SetOutput(uint16_t freq, uint8_t duty);
#endif
