sed -i '/\/\* USER CODE BEGIN 1 \*\//,$d' Core/Src/usart.c
cat << 'HEREDOC' >> Core/Src/usart.c
/* USER CODE BEGIN 1 */

/* UART接收缓冲 */
#define UART_RX_BUF_SIZE 20
static uint8_t g_uart_rx_buf[UART_RX_BUF_SIZE];
static uint8_t g_uart_rx_index = 0;
static uint8_t g_uart_rx_data = 0;

/* 初始化UART并启动接收中断 */
void UART_Init(void)
{
    g_uart_rx_index = 0;
    
    /* 启动UART接收中断（接收1个字节后触发中断） */
    HAL_UART_Receive_IT(&huart1, &g_uart_rx_data, 1);
}

/* 发送单个字符 */
void UART_SendChar(uint8_t ch)
{
    HAL_UART_Transmit(&huart1, &ch, 1, 100);
}

/* 发送字符串 */
void UART_SendString(char *str)
{
    while (*str) {
        UART_SendChar(*str);
        str++;
    }
}

void UART_HandleCmd(char *cmd)
{
    char old_pwd[4];
    char new_pwd[4];
    if (strlen(cmd) >= 7 && cmd[3] == '-') {
        strncpy(old_pwd, cmd, 3);
        old_pwd[3] = '\0';
        strncpy(new_pwd, cmd + 4, 3);
        new_pwd[3] = '\0';
        if (strcmp(old_pwd, current_pwd) == 0) {
            strcpy(current_pwd, new_pwd);
            UART_SendString("Password changed!\r\n");
        } else {
            UART_SendString("Wrong password!\r\n");
        }
    }
}

/* UART接收中断回调函数 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        /* 将接收到的字符存入缓冲 */
        g_uart_rx_buf[g_uart_rx_index++] = g_uart_rx_data;
        
        /* 缓冲满或接收到结束符('\r') */
        if (g_uart_rx_index >= UART_RX_BUF_SIZE - 1 || 
            g_uart_rx_data == '\r') {
            
            g_uart_rx_buf[g_uart_rx_index] = '\0';
            
            /* 处理接收到的命令 */
            UART_HandleCmd((char *)g_uart_rx_buf);
            
            /* 重置缓冲 */
            g_uart_rx_index = 0;
        }
        
        /* 继续接收下一个字符 */
        HAL_UART_Receive_IT(&huart1, &g_uart_rx_data, 1);
    }
}

/* UART处理函数（可选，用于超时检测） */
void UART_Process(void)
{
    /* 如果有接收缓冲数据，但超过1秒没有新数据，则重置 */
    static uint32_t last_rx_time = 0;
    
    if (g_uart_rx_index > 0) {
        if (uwTick - last_rx_time > 1000) {
            /* 接收超时，清空缓冲 */
            g_uart_rx_index = 0;
        }
    } else {
        last_rx_time = uwTick;
    }
}

/* USER CODE END 1 */
HEREDOC
