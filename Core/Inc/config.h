// Core/Inc/config.h

#ifndef __CONFIG_H
#define __CONFIG_H

/* ============= 密码配置 ============= */
#define Defaultnum              "123"
#define PASSWORD_LENGTH         3
#define MAX_PASSWORD_ATTEMPTS   3

/* ============= 时间间隔(ms) ============= */
#define LCD_REFRESH_INTERVAL    100     // LCD刷新间隔
#define UART_PROCESS_INTERVAL   50      // 串口处理间隔

/* ============= PWM/脉冲配置 ============= */
#define NORMAL_FREQ             1000    // 正常频率(Hz)
#define NORMAL_DUTY             50      // 正常占空比(%)
#define VERIFY_FREQ             2000    // 验证成功时的频率(Hz)
#define VERIFY_DUTY             10      // 验证成功时的占空比(%)
#define VERIFY_DURATION         5000    // 验证成功输出时长(ms)

/* ============= LED配置 ============= */
#define LED1_ON_DURATION        5000    // LD1点亮时长(ms)
#define LED2_ALARM_INTERVAL     100     // LD2闪烁间隔(ms)
#define LED2_ALARM_DURATION     5000    // LD2报警时长(ms)

/* ============= LCD行定义 ============= */
#define LCD_LINE_TITLE          Line0
#define LCD_LINE_PSW_1          Line2
#define LCD_LINE_PSW_2          Line4
#define LCD_LINE_FREQ           Line2
#define LCD_LINE_DUTY           Line4

#endif
