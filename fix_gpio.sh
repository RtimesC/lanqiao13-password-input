sed -i '/\/\* USER CODE BEGIN 2 \*\//a\
#include "main.h"\n#include "config.h"\n#include "led.h"\n#include "pwm.h"\n#include "lcd.h"\n#include "key.h"\n#include <string.h>\n#include <stdio.h>\n#include <stdint.h>\n' Core/Src/gpio.c
