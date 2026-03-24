#ifndef __APP_H
#define __APP_H

#include "main.h"

/* ============= 应用状态枚举 ============= */
typedef enum {
    APP_STATE_PSD_INPUT = 0,        // 密码输入界面
    APP_STATE_STA_OUTPUT = 1        // 输出状态界面
} AppStateType;

/* ============= 密码管理结构 ============= */
typedef struct {
    char current_password[4];       // 当前密码（3位+\0）
    char input_buffer[4];           // 输入缓冲（3位+\0）
    uint8_t input_count;            // 已输入位数
    uint8_t error_count;            // 连续错误次数
    uint32_t last_input_time;       // 上次输入时间
} PasswordType;

/* ============= PWM输出结构 ============= */
typedef struct {
    uint16_t frequency;             // 频率(Hz)
    uint8_t duty_cycle;             // 占空比(%)
    uint32_t start_time;            // 开始时间
    uint32_t duration;              // 持续时长(ms)，0表示持续
    uint8_t is_active;              // 是否活跃
} PWMOutputType;

/* ============= 应用数据结构 ============= */
typedef struct {
    AppStateType state;             // 当前界面状态
    PasswordType password;          // 密码管理
    PWMOutputType pwm;              // PWM输出
    
    uint32_t led1_on_time;          // LD1点亮的时间
    uint32_t led2_alarm_time;       // LD2报警的时间
    uint8_t led2_alarm_state;       // LD2闪烁状态
    
    uint32_t last_lcd_update;       // 上次LCD更新时间
} AppDataType;

/* ============= 外部接口 ============= */
void APP_Init(void);                // 初始化应用
void APP_Process(void);             // 主处理函数
void APP_HandleKeyInput(uint8_t key_id);  // 处理按键
void APP_HandleUARTPassword(char *cmd);   // 处理UART密码修改

#endif
