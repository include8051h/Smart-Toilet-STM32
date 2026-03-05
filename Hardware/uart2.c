 /******************************************************************************
 * @file    uart2.c
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/13
 * @brief   串口2相关功能具体实现代码源文件
 * @version 无
 * @note    无
 ******************************************************************************/



#include "stm32f10x.h"
#include <stdio.h>



//串口2初始化函数
void USART2_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;//定义一个GPIO初始化结构体，用于存储各种配置参数
    USART_InitTypeDef USART_InitStruct;//定义一个USART初始化结构体，用于存储各种配置参数

    //使能时钟（GPIOA + USART2）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//启动GPIOA的RCC时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); //启动串口2的RCC时钟

    //配置 PA2 为复用推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;	//选中2号引脚
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;	//选择复用推挽输出模式
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;	//速度选择50MHz
    GPIO_Init(GPIOA, &GPIO_InitStruct);	//初始化上述配置参数

    //配置 PA3 为浮空输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;	//选中3号引脚
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//选中浮空输入模式
    GPIO_Init(GPIOA, &GPIO_InitStruct);	//初始化上述配置参数

    //配置 USART2
    USART_InitStruct.USART_BaudRate = 115200;	//波特率设为115200
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;	//数据帧长度设为8位
    USART_InitStruct.USART_StopBits = USART_StopBits_1;	//停止位设为1位
    USART_InitStruct.USART_Parity = USART_Parity_No;	//不使用奇偶校验
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//禁用硬件流控
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//使能USART的发送和接收模式
    USART_Init(USART2, &USART_InitStruct);	//初始化上述配置参数

    //使能 USART2
    USART_Cmd(USART2, ENABLE);
}

// 重定向 printf 到 USART2
int fputc(int ch, FILE *f) {
    // 等待发送缓冲区空
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
    // 发送一个字节
    USART_SendData(USART2, (uint8_t)ch);
    return ch;
}
