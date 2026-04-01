// Core/Src/app.c

#include "app.h"
#include "key.h"
#include "led.h"
#include "usart.h"
#include "pwm.h"
#include "lcd.h"
#include "config.h"
#include <string.h>
#include <stdio.h>

/* ============= 全局应用数据 ============= */
static AppDataType g_app = {0};
static uint8_t g_lcd_buf[20];

/* ============= 前向声明（必须！） ============= */
static void Success(void);
static void Fail(void);
static void APP_UpdatePWM(uint32_t current_tick);
static void APP_UpdateLED(uint32_t current_tick);
static void APP_UpdateDisplay(uint32_t current_tick);//静态函数实现内部细节，防止意外调用

/* ============= 初始化 ============= */
void APP_Init(void)
{
    /* 初始化密码 */
    strcpy((char *)g_app.password.current_password, Defaultnum);
    strcpy((char *)g_app.password.input_buffer, "@@@");
    g_app.password.input_count = 0;
    g_app.password.error_count = 0;
    
    /* 初始化PWM */
    g_app.pwm.frequency = NORMAL_FREQ;      // 1KHz
    g_app.pwm.duty_cycle = NORMAL_DUTY;     // 50%
    g_app.pwm.duration = 0;                 // 持续输出
    g_app.pwm.is_active = 1;
    PWM_SetOutput(NORMAL_FREQ, NORMAL_DUTY);
    
    /* 进入密码输入界面 */
    g_app.state = APP_STATE_PSD_INPUT;
    g_app.last_lcd_update = 0;
    
    LCD_Clear(Black);
}

/* ============= 主处理函数 ============= */
void APP_Process(void)
{
    uint32_t current_tick = uwTick;
    
    /* 处理LED */
    APP_UpdateLED(current_tick);
    
    /* 处理PWM定时 */
    APP_UpdatePWM(current_tick);
    
    /* 定时刷新LCD */
    if (current_tick - g_app.last_lcd_update >= LCD_REFRESH_INTERVAL) {
        APP_UpdateDisplay(current_tick);
        g_app.last_lcd_update = current_tick;
    }
}

/* ============= 按键处理 ============= */
void APP_HandleKeyInput(uint8_t key_id)
{
    /* 仅在密码输入界面处理按键 */
    if (g_app.state != APP_STATE_PSD_INPUT)
        return;
    
    if (key_id >= 1 && key_id <= 3) {
        /* B1/B2/B3: 调整密码位 */
        uint8_t digit = g_app.password.input_buffer[key_id - 1] - '0';
        if (digit == 0xFF) digit = 0;  // '@'转换为0
        
        digit = (digit + 1) % 10;
        g_app.password.input_buffer[key_id - 1] = '0' + digit;
    }
    else if (key_id == 4) {
        /* B4: 密码确认 */
        if (strncmp((char *)g_app.password.input_buffer, 
                    (char *)g_app.password.current_password, 
                    PASSWORD_LENGTH) == 0) {
            /* ✓ 密码正确 */
            Success();
        } else {
            /* ✗ 密码错误 */
            Fail();
        }
    }
}

/* ============= 密码验证成功 ============= */
static void Success(void)
{
    /* 设置PWM为2KHz, 10%占空比，持续5秒 */
    g_app.pwm.frequency = VERIFY_FREQ;
    g_app.pwm.duty_cycle = VERIFY_DUTY;
    g_app.pwm.start_time = uwTick;
    g_app.pwm.duration = VERIFY_DURATION;
    g_app.pwm.is_active = 1;
    PWM_SetOutput(VERIFY_FREQ, VERIFY_DUTY);
    
    /* 点亮LD1（5秒） */
    LED_SetLD1OnTime(LED1_ON_DURATION);
    
    /* 切换到输出状态界面 */
    g_app.state = APP_STATE_STA_OUTPUT;
    g_app.password.error_count = 0;
}

/* ============= 密码验证失败 ============= */
static void Fail(void)
{
    /* 密码重置为@ */
    strcpy((char *)g_app.password.input_buffer, "@@@");
    g_app.password.input_count = 0;
    
    /* 错误计数 */
    g_app.password.error_count++;
    
    /* 3次错误触发LED2报警 */
    if (g_app.password.error_count >= MAX_PASSWORD_ATTEMPTS) {
        LED_SetLD2AlarmTime(LED2_ALARM_DURATION);
        g_app.password.error_count = 0;
    }
}

/* ============= UART密码修改 ============= */
void APP_HandleUARTPassword(char *cmd)
{
    /* 指令格式: "123-789" (7个字符) */
    char old_pwd[4], new_pwd[4];
    
    if (strlen(cmd) != 7 || cmd[3] != '-')
        return;
    
    strncpy(old_pwd, cmd, 3);
    old_pwd[3] = '\0';
    strncpy(new_pwd, cmd + 4, 3);
    new_pwd[3] = '\0';
    
    /* 验证旧密码 */
    if (strcmp(old_pwd, (char *)g_app.password.current_password) == 0) {
        strcpy((char *)g_app.password.current_password, new_pwd);
        UART_SendString("Password changed!\r\n");
    } else {
        UART_SendString("Wrong password!\r\n");
    }
}

/* ============= PWM更新 ============= */
static void APP_UpdatePWM(uint32_t current_tick)
{
    if (!g_app.pwm.is_active)
        return;
    
    /* 检查持续时间是否超过 */
    if (g_app.pwm.duration > 0) {
        if (current_tick - g_app.pwm.start_time >= g_app.pwm.duration) {
            /* 恢复为1KHz, 50% */
            g_app.pwm.frequency = NORMAL_FREQ;
            g_app.pwm.duty_cycle = NORMAL_DUTY;
            g_app.pwm.duration = 0;
            PWM_SetOutput(NORMAL_FREQ, NORMAL_DUTY);
            
            /* 切回密码输入界面 */
            g_app.state = APP_STATE_PSD_INPUT;
            strcpy((char *)g_app.password.input_buffer, "@@@");
            g_app.password.input_count = 0;
        }
    }
}

/* ============= LED更新 ============= */
static void APP_UpdateLED(uint32_t current_tick)
{
    /* LD1处理（点亮5秒后熄灭） */
    if (g_app.led1_on_time > 0) {
        if (current_tick - g_app.led1_on_time >= LED1_ON_DURATION) {
            LED_TurnOff(LED1);
            g_app.led1_on_time = 0;
        }
    }
    
    /* LD2报警处理（闪烁5秒） */
    if (g_app.led2_alarm_time > 0) {
        if (current_tick - g_app.led2_alarm_time >= LED2_ALARM_DURATION) {
            LED_TurnOff(LED2);
            g_app.led2_alarm_time = 0;
        } else {
            /* 每0.1秒切换一次 */
            uint32_t elapsed = current_tick - g_app.led2_alarm_time;
            uint8_t new_state = (elapsed / LED2_ALARM_INTERVAL) % 2;
            
            if (new_state != g_app.led2_alarm_state) {
                g_app.led2_alarm_state = new_state;
                if (new_state) {
                    LED_TurnOn(LED2);
                } else {
                    LED_TurnOff(LED2);
                }
            }
        }
    }
}

/* ============= LCD显示更新 ============= */
static void APP_UpdateDisplay(uint32_t current_tick)
{
    if (g_app.state == APP_STATE_PSD_INPUT) {
        /* 密码输入界面 */
        LCD_DisplayStringLine(LCD_LINE_TITLE, (uint8_t *)"      PSD        ");
        
        sprintf((char *)g_lcd_buf, "   %c %c %c         ", 
                g_app.password.input_buffer[0],
                g_app.password.input_buffer[1],
                g_app.password.input_buffer[2]);
        LCD_DisplayStringLine(LCD_LINE_PSW_1, g_lcd_buf);
    }
    else if (g_app.state == APP_STATE_STA_OUTPUT) {
        /* 输出状态界面 */
        LCD_DisplayStringLine(LCD_LINE_TITLE, (uint8_t *)"      STA        ");
        
        sprintf((char *)g_lcd_buf, "   F:%dHz         ", g_app.pwm.frequency);
        LCD_DisplayStringLine(LCD_LINE_FREQ, g_lcd_buf);
        
        sprintf((char *)g_lcd_buf, "   D:%d%%          ", g_app.pwm.duty_cycle);
        LCD_DisplayStringLine(LCD_LINE_DUTY, g_lcd_buf);
    }
}
