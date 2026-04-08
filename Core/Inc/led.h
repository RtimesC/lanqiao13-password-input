#ifndef __LED_H
#define __LED_H
#include "stm32g4xx_hal.h"
#include <stdint.h>
#define LED1 0
#define LED2 1
void LED_Init(void);
void LED_TurnOn(uint8_t led_id);
void LED_TurnOff(uint8_t led_id);
#endif
