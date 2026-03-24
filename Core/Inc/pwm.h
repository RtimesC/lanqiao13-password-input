#ifndef __PWM_H
#define __PWM_H

#include "stdint.h"

void PWM_Init(void);
void PWM_SetOutput(uint16_t freq, uint8_t duty);

#endif
