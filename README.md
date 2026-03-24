## 📚 项目代码详细分析

### **项目概述**

这是一个**STM32G4嵌入式密码输入系统**，基于蓝桥杯13届赛题实现。项目核心功能包括：
- 🔐 **密码输入与验证**
- 💡 **PWM脉冲输出**
- 🔴 **LED指示与报警**
- 📺 **LCD显示界面**
- ⌨️ **按键扫描处理**
- 🔌 **UART串口通信**

---

## 🏗️ 整体架构

```
┌─────────────────────────────────────┐
│        Main Loop (main.c)           │
│  - KEY_Process()    ← 按键扫描      │
│  - UART_Process()   ← 串口接收      │
│  - APP_Process()    ← 应用处理      │
└────────────┬────────────────────────┘
             │
    ┌────────┴─────────┬────────────┬──────────────┐
    │                  │            │              │
    ▼                  ▼            ▼              ▼
┌────────┐      ┌──────────┐  ┌─────────┐  ┌───────────┐
│ KEY    │      │  APP     │  │  LED    │  │    PWM    │
│Module  │─────→│ Manager  │→ │ Control │  │ Generate  │
└────────┘      └──────────┘  └─────────┘  └───────────┘
                       │
                       ▼
                   ┌────────┐
                   │  LCD   │
                   │Display │
                   └────────┘
```

---

## 🔑 核心模块详解

### **1️⃣ 密码验证流程 (app.c)**

#### **核心状态机**

```c
应用有2个状态：
├─ APP_STATE_PSD_INPUT   (0) → 密码输入界面
└─ APP_STATE_STA_OUTPUT  (1) → 验证成功输出状态
```

#### **密码输入逻辑**

| 按键 | 功能 | 实现 |
|------|------|------|
| B1   | 第1位 +1 | `digit = (digit + 1) % 10` |
| B2   | 第2位 +1 | 同B1逻辑 |
| B3   | 第3位 +1 | 同B1逻辑 |
| B4   | 确认 | `strncmp()` 密码比对 |

**初始密码显示**: `@@@` → 代表`000`

**按键处理示例**:
```c
按键B1被按下:
  input_buffer[0] = '0' + ((('0'+1-'0')+1)%10) = '2'
  @@@  →  2@@
```

#### **验证成功流程**

```
密码验证正确 (strncmp成功)
    │
    ▼
APP_PasswordVerifySuccess()
    │
    ├─ PWM切换: 1KHz/50% → 2KHz/10%
    ├─ PWM时长: 5秒
    ├─ LED1点亮: 5秒
    ├─ 状态切换: PSD_INPUT → STA_OUTPUT
    └─ 错误计数清零
    
5秒后自动恢复:
    ├─ PWM恢复: 2KHz/10% → 1KHz/50%
    ├─ LED1熄灭
    ├─ 状态切换: STA_OUTPUT → PSD_INPUT
    └─ 密码重置: @@@
```

#### **验证失败流程**

```
密码验证失败 (strncmp失败)
    │
    ▼
APP_PasswordVerifyFail()
    │
    ├─ 密码重置为 @@@
    ├─ 错误计数 +1 (error_count++)
    │
    ├─ if (error_count >= 3)
    │   └─ LD2报警5秒（每0.1s闪烁）
    │   └─ 错误计数清零
    │
    └─ 继续等待输入
```

---

### **2️⃣ 按键扫描 (key.c)**

#### **扫描策略：防抖处理**

```c
KEY_Process()执行流程：
├─ 检查扫描间隔: if (uwTick - g_key_tick < 20ms) return;
│  └─ 20ms防抖周期，避免频繁扫描
│
├─ 读取4个GPIO引脚的当前状态
│  ├─ B1: GPIOB_PIN_0  (取反: !HAL_GPIO_ReadPin())
│  ├─ B2: GPIOB_PIN_1
│  ├─ B3: GPIOB_PIN_2
│  └─ B4: GPIOA_PIN_0
│
├─ 边沿检测（下降沿 → 按键按下）
│  ├─ if (key_current[i] && !g_key_old[i])
│  │  └─ 当前为按下 且 上一次为释放 → 检测到按键按下
│  │
│  └─ 调用 APP_HandleKeyInput(i + 1)
│
└─ 更新上一次状态: g_key_old[i] = key_current[i]
```

**GPIO逻辑**:
- 按键未按下: GPIO=HIGH → 取反后=0
- 按键已按下: GPIO=LOW → 取反后=1
- 因此: `!HAL_GPIO_ReadPin()` = 按键状态

---

### **3️⃣ LED控制 (led.c)**

#### **LED驱动原理**

```c
void LED_TurnOn(uint8_t led_id)  // 打开第led_id个LED
{
    uint8_t led_value = (1 << led_id);   // 位操作: 第led_id位置1
    LED_Disp(led_value);                 // 调用gpio层函数输出
}

示例：LED_TurnOn(1) = LED_Disp(0b00000010)
      位 7 6 5 4 3 2 1 0
      值 0 0 0 0 0 0 1 0  → LED1打开
```

#### **LD2报警闪烁实现**

在 `APP_UpdateLED()` 中实现：

```c
uint32_t elapsed = current_tick - g_app.led2_alarm_time;
uint8_t new_state = (elapsed / 100) % 2;  // 100ms周期切换

周期示例：
时间    elapsed  new_state
0ms      0        0(关)
100ms    100      1(开)  ← 切换
200ms    200      0(关)  ← 切换
300ms    300      1(开)  ← 切换
...
5000ms   5000     关闭报警
```

---

### **4️⃣ PWM脉冲输出 (pwm.c)**

#### **PWM工作模式**

```
正常模式:
频率: 1KHz
占空比: 50%
时长: 持续输出

验证成功 (5秒内):
频率: 2KHz
占空比: 10%
时长: 5000ms
```

#### **PWM定时管理** (app.c中)

```c
APP_UpdatePWM(current_tick)
{
    if (!pwm.is_active) return;
    
    if (pwm.duration > 0) {
        elapsed = current_tick - pwm.start_time;
        
        if (elapsed >= pwm.duration) {
            // 5秒到期，恢复正常模式
            pwm.frequency = 1000;
            pwm.duty_cycle = 50;
            pwm.duration = 0;
            PWM_SetOutput(1000, 50);
            
            // 状态恢复
            state = PSD_INPUT;
            input_buffer = "@@@";
        }
    }
}
```

---

### **5️⃣ LCD显示管理**

#### **双界面显示切换**

**密码输入界面**:
```
Line0: "      PSD        "
Line2: "   X X X         "  (X=当前输入数字或@)
```

**验证成功界面**:
```
Line0: "      STA        "
Line2: "   F:2000Hz      "
Line4: "   D:10%         "
```

#### **显示刷新策略**

```c
if (current_tick - g_app.last_lcd_update >= 100ms) {
    APP_UpdateDisplay(current_tick);  // 100ms刷新一次，减少闪烁
    g_app.last_lcd_update = current_tick;
}
```

---

### **6️⃣ UART通信**

#### **密码修改指令**

```
指令格式: "123-789\r\n"
说明: 旧密码123 → 新密码789

处理流程:
1. UART接收 "123-789"
2. 验证旧密码 strcmp("123", current_pwd)
3. 成功 → 更新密码 + 回复 "Password changed!\r\n"
4. 失败 → 回复 "Wrong password!\r\n"
```

---

## 🔄 主循环执行流程

```c
int main(void) {
    /* 初始化所有外设 */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();      // GPIO初始化
    MX_TIM2_Init();      // 定时器初始化(PWM)
    MX_USART1_UART_Init();  // 串口初始化
    
    /* 初始化应用模块 */
    LCD_Init();
    KEY_Init();
    LED_Init();
    APP_Init();          // 状态 = PSD_INPUT, 密码 = "@@@"
    
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);  // 启动PWM输出
    
    /* ============ 无限循环 ============ */
    while (1) {
        ┌─────────────────────────────────┐
        │ 1. KEY_Process()  (20ms)        │
        │    - 读取4个GPIO引脚            │
        │    - 边沿检测                   │
        │    - 调用 APP_HandleKeyInput()  │
        └─────────────────────────────────┘
                    │
        ┌───────────▼──────────────────────┐
        │ 2. UART_Process()  (50ms)        │
        │    - 检查UART接收缓冲            │
        │    - 解析密码修改指令            │
        │    - 调用 APP_HandleUARTPassword│
        └────────────────────────────────┘
                    │
        ┌───────────▼──────────────────────┐
        │ 3. APP_Process()  (实时)         │
        │    - APP_UpdateLED()             │
        │      └─ 处理LED1/LED2定时        │
        │    - APP_UpdatePWM()             │
        │      └─ 处理PWM时长倒计时        │
        │    - APP_UpdateDisplay() (100ms) │
        │      └─ 刷新LCD显示             │
        └────────────────────────────────┘
}
```

---

## ⏱️ 时间管理

系统使用 `uwTick` (HAL系统时钟计数器，单位ms) 实现时间管理：

```
事件           时间间隔    处理位置
──────────────────────────────────
按键防抖       20ms       KEY_Process()
UART处理       50ms       UART_Process()
LCD刷新        100ms      APP_UpdateDisplay()
LED1点亮       5000ms     APP_UpdateLED()
LED2闪烁周期   100ms      APP_UpdateLED()
LD2报警时长    5000ms     APP_UpdateLED()
PWM持续时长    5000ms     APP_UpdatePWM()
```

---

## 🎯 工作流程示例

### **场景：用户输入密码"123"并验证**

```
初始状态: password="@@@", error_count=0, state=PSD_INPUT

① 用户按下B1
   KEY_Process() → APP_HandleKeyInput(1)
   input_buffer[0] = (0+1)%10 = '1'
   LCD显示: "1@@"

② 用户按下B2
   KEY_Process() → APP_HandleKeyInput(2)
   input_buffer[1] = (0+1)%10 = '2'
   LCD显示: "12@"

③ 用户按下B3
   KEY_Process() → APP_HandleKeyInput(3)
   input_buffer[2] = (0+1)%10 = '3'
   LCD显示: "123"

④ 用户按下B4（确认）
   KEY_Process() → APP_HandleKeyInput(4)
   strncmp("123", "123") == 0 ✓
   APP_PasswordVerifySuccess()
   
   ├─ PWM: 2KHz/10% (5秒)
   ├─ LED1: 点亮 (5秒)
   ├─ state = STA_OUTPUT
   └─ LCD: 显示"STA" + "F:2000Hz" + "D:10%"

⑤ 5秒后自动恢复
   APP_UpdatePWM()触发时间判断
   ├─ PWM恢复: 1KHz/50%
   ├─ LED1熄灭
   ├─ state = PSD_INPUT
   └─ LCD: 显示"PSD" + "@@@"
```

---

## 🛡️ 错误处理

### **错误流程示例**

```
初始: password="@@@", error_count=0

① 用户输入"111"，按B4验证
   strncmp("111", "123") != 0 ✗
   APP_PasswordVerifyFail()
   └─ error_count = 1
   └─ password重置为"@@@"

② 再错一次
   error_count = 2
   password = "@@@"

③ 再错一次（第3次）
   error_count = 3 >= MAX_PASSWORD_ATTEMPTS
   ├─ LED_SetLD2AlarmTime() → LED2报警5秒
   ├─ 闪烁间隔: 100ms切换一次
   └─ error_count清零
   
④ 5秒后停止闪烁
   APP_UpdateLED()检测到时间到期
   └─ LED2熄灭
```

---

## 📊 系统配置总览

```c
配置文件: config.h

密码配置:
  默认密码: "123"
  密码长度: 3位
  最大错误: 3次

时间配置 (ms):
  按键扫描: 20
  LCD刷新: 100
  串口处理: 50

PWM配置:
  正常: 1KHz, 50%占空比
  验证: 2KHz, 10%占空比, 5秒持续

LED配置:
  LD1亮: 5秒
  LD2闪: 5秒, 100ms周期
```

---

## 🎯 总结

该项目采用**事件驱动 + 状态机**的设计模式：

1. **按键扫描** → 触发密码处理
2. **状态机管理** → PSD_INPUT ↔ STA_OUTPUT
3. **定时器控制** → LED/PWM时序精确管理
4. **LCD实时更新** → 用户友好的界面反馈
5. **UART扩展** → 支持远程密码修改

整个系统通过主循环周期性调用各模块的`Process()`函数，利用时间戳实现各功能的异步协作，避免阻塞式设计！
# Project Overview
This project is designed to implement a secure and user-friendly password input mechanism. It focuses on ensuring that sensitive information is handled appropriately while providing a seamless experience for users.

# Architecture
The architecture consists of several interconnected modules including the input handler, encryption modules, and a user interface. Each component is designed with security and efficiency in mind.

# Core Modules
1. **Input Handler**: Captures user input securely and processes it.
2. **Encryption Module**: Implements robust algorithms to encrypt user passwords before they are stored or transmitted.
3. **User Interface**: Provides an interactive experience, allowing users to enter passwords with additional features like visibility toggle.

# Main Loop Flow
The main loop is structured to handle user input, process commands, and manage the state of the application. Key steps include:
- Initializing the application.
- Waiting for user input.
- Processing commands based on input.
- Updating the UI promptly.

# Timing Management
The system manages timing to ensure responsiveness, using techniques such as event-driven programming to minimize latency and enhance user experience.

# Workflow Examples
1. **Basic Password Input**: Users simply type their password, which is immediately encrypted and transmitted securely.
2. **Visibility Toggle**: Users can toggle the visibility of their password input, allowing them to verify their entries while maintaining security.

# Error Handling
Error handling is integrated within each module, ensuring that exceptions are caught and handled gracefully without crashing the application. Common errors include invalid input and encryption failures, which are logged for debugging purposes.

# System Configuration
System configuration can be adjusted through a settings file where users can specify parameters like encryption strength, UI themes, and timeout intervals. Default values are provided for ease of use.

# Summary
This project provides a comprehensive solution for secure password input, emphasizing user experience without sacrificing security. Through a well-structured architecture and thoughtful design, it ensures that sensitive information is protected at all stages of user interaction.  
