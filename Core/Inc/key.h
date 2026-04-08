#ifndef __KEY_H
#define __KEY_H

#include "stm32g4xx_hal.h"
#include <stdint.h>

extern uint8_t key_down;

void KEY_Init(void);
void key_read(void);

#endif
