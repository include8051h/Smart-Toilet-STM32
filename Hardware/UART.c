 /******************************************************************************
 * @file    UART.c
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/13
 * @brief   发送或接收ESP8266-01S功能具体实现代码源文件
 * @version 无
 * @note    无
 ******************************************************************************/



#include "UART.h"

// 全局接收缓冲区
#define RX_BUFFER_SIZE 256	//缓冲区空间大小
uint8_t rx_buffer[RX_BUFFER_SIZE];	// 全局接收缓冲区

uint16_t rx_head = 0;	//接收信息头部
uint16_t rx_tail = 0;	//接收信息尾部



//获取接收信息头部
uint16_t get_rx_head(void)
{
	return rx_head;	//返回接收信息头部
}



//获取接收信息尾部
uint16_t get_rx_tail(void)
{
	return rx_tail;	//返回接收信息尾部
}



//重置接收消息头部和尾部
void reset_rx_headTail(void)
{
	//将接收信息的头部和尾部置0
	rx_head = 0;
	rx_tail = 0;
}



//清空接收缓冲区
void reset_rx_buffer(void)
{
	memset(rx_buffer, 0, sizeof(rx_buffer));//memset函数将接收缓冲区清零
}

// 初始化ESP8266的串口（USART1）
void USART1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;	//定义一个GPIO初始化结构体，用于存储各种初始化配置参数
    USART_InitTypeDef USART_InitStruct;	//定义一个USART初始化结构体，用于存储各种初始化配置参数
    NVIC_InitTypeDef NVIC_InitStruct;	//定义一个NVIC初始化结构体，用于存储各种初始化配置参数

    //使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    //配置 PA9 (U1 TX) - 复用推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;	//选择9号引脚
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;	//选择复用推挽输出
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;	//速度选择50MHz
    GPIO_Init(GPIOA, &GPIO_InitStruct);	//初始化上述配置参数

    //配置 PA10 (U1 RX) - 浮空输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;	//引脚选择10号引脚
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//模式选择浮空输入模式
    GPIO_Init(GPIOA, &GPIO_InitStruct);	//初始化上述配置参数

    //配置 USART1
    USART_InitStruct.USART_BaudRate = 115200;	//波特率选择115200
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;	//设置数据帧的字长为8位。
    USART_InitStruct.USART_StopBits = USART_StopBits_1;	//设置停止位为1位
    USART_InitStruct.USART_Parity = USART_Parity_No;	//不使用奇偶校验
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//禁用硬件流控
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//使能USART的发送和接收模式
    USART_Init(USART1, &USART_InitStruct);	//初始化上述配置参数

    //使能接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //RXNE中断

    //配置 NVIC
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;	//选择中断通道
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;	//抢占优先级设置为1
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;	//子优先级设置为2
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;	//中断使能
    NVIC_Init(&NVIC_InitStruct);	//初始化上述配置参数

    //使能 USART1
    USART_Cmd(USART1, ENABLE);
}

//读取一行数据
uint16_t ReadLine(char* buffer, uint16_t max_len) {
    uint16_t len = 0;	//存储数据长度的临时变量
    while (rx_tail != rx_head && len < max_len - 1) {//如果数据没有读完且读取的数据长度小于最长数据长度
			//就一直读取下去
        char ch = rx_buffer[rx_tail];	//读取新的一位数据
        rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;	//更新数据尾部的位置

        if (ch == '\n' || ch == '\r') {//如果读取到换行符
            break;//退出循环
        }
        buffer[len++] = ch;	//存储接收到的数据
    }
    buffer[len] = '\0';	//数据完毕后在其尾部加上字符串结束符
    return len;	//返回数据长度
}

// 判断缓冲区是否为空
static inline uint8_t ring_buffer_empty(void) {
    return (rx_head == rx_tail);	//返回接收数据头部与尾部的位置是否相同，
	//如果相同即缓冲区为空返回1，不同即缓冲区不为空返回0
}

// 判断缓冲区是否已满
//static inline uint8_t ring_buffer_full(void) {
//    return ((rx_head + 1) % RX_BUFFER_SIZE == rx_tail);
//}

// 写入一个字节到缓冲区，由中断调用
//static void ring_buffer_put(uint8_t ch) {
//    if (!ring_buffer_full()) {
//        rx_buffer[rx_head] = ch;
//        rx_head = (rx_head + 1) % RX_BUFFER_SIZE;
//    }
//}

// 从缓冲区读取一行，遇到 '\n' 或超长则返回
uint16_t ESP8266_ReadLine(char* buffer, uint16_t max_len) {
    if (max_len == 0) return 0;	//如果要读取的数据长度为0，直接返回
    
    uint16_t i = 0;	//迭代计数器
    uint8_t ch;	//用数据存储中转的临时变量
    
    while (i < max_len - 1) 
			{
        if (ring_buffer_empty()) {	//如果数据缓冲区为空
            break; //跳出循环
        }
        
        //进入临界区
        taskENTER_CRITICAL();
				//从接收缓冲区尾部读取数据
        ch = rx_buffer[rx_tail];
				//更新尾部位置
        rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;
				//进入临界区
        taskEXIT_CRITICAL();
        
				//将读取到的新数据加入buffer尾部
        buffer[i++] = ch;
        
        // 如果是换行符，则认为一行结束
        if (ch == '\n') {
            break;//跳出数据读取循环
        }
    }
    
    buffer[i] = '\0'; // 字符串结尾加上结束符
    return i;//返回读取数据的长度
}



//通过串口向ESP8266-01S发送字符串函数
void ESP8266_SendString(char *str) {
	while(*str) {//如果没到发送数据末尾
		USART_SendData(USART1, *str++);	//串口发送数据，串口选择串口1，发送该位数据后自动换下一位
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);//串口发送完成标志位
    }
}

// 发送AT指令功能具体实现函数
void ESP8266_SendAT(char *at_cmd) 
{
	ESP8266_SendString(at_cmd);//发送AT指令
  ESP8266_SendString("\r\n");  // AT指令必须以回车换行结束
}


/*
void Parse_OneNET_Property(const char* json_str) {
    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL) {
        printf("[ERROR] JSON parse failed!\n");
        LED_OFF();
        return;
    }

    // 获取 params 对象
    cJSON *params = cJSON_GetObjectItem(root, "params");
    if (params == NULL || params->type != cJSON_Object) {  // ← 现在这行能正常工作！
        printf("[ERROR] 'params' not found or not an object!\n");
        LED_OFF();
        cJSON_Delete(root);
        return;
    }

    // 获取 wind 字段
    cJSON *wind = cJSON_GetObjectItem(params, "wind");
    if (wind == NULL) {
        printf("[ERROR] 'wind' not found!\n");
        LED_OFF();
        cJSON_Delete(root);
        return;
    }

    // 控制风扇
    if (cJSON_IsTrue(wind)) {          // cJSON v1.7+ 推荐用这个宏
        Wind_ON();
        printf("[INFO] Fan ON\n");
    } else if (cJSON_IsFalse(wind)) {
        Wind_OFF();
        printf("[INFO] Fan OFF\n");
    } else {
        printf("[WARN] 'wind' is not boolean\n");
    }

    cJSON_Delete(root);
}*/



//串口1中断服务函数
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {//如果串口中断标志位置一
        uint8_t ch = USART_ReceiveData(USART1); // 自动清除 RXNE 标志

        
        rx_buffer[rx_head] = ch;//存储接收数据
        rx_head = (rx_head + 1) % RX_BUFFER_SIZE;//移动存储新数据的位置

        // 可选：回显
        // USART_SendData(USART1, ch);
    }
}
