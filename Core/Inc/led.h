#ifndef __LED_H
#define __LED_H

#include <stdint.h>

#define LED1  0
#define LED2  1
#define LED3  2
#define LED4  3
#define LED5  4
#define LED6  5
#define LED7  6
#define LED8  7

void LED_Init(void);
void LED_TurnOn(uint8_t led_id);
void LED_TurnOff(uint8_t led_id);
void LED_SetLD1OnTime(uint32_t duration);
void LED_SetLD2AlarmTime(uint32_t duration);

#endif
