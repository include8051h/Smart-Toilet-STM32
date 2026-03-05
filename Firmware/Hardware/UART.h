 /******************************************************************************
 * @file    UART.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/13
 * @brief   发送或接收ESP8266-01S功能具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"                  // Device header
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cJSON.h"
#include "Wind.h"
#include <stdio.h>
#include "LED.h"



/**
 * @func			 	get_rx_head
 * @brief      	获取接收信息头部
 * @param		  	无
 * @retval     	uint16_t
 * @note       	无
 */
uint16_t get_rx_head(void);



/**
 * @func			 	get_rx_tail
 * @brief      	获取接收信息尾部
 * @param		  	无
 * @retval     	uint16_t
 * @note       	无
 */
uint16_t get_rx_tail(void);



/**
 * @func			 	reset_rx_headTail
 * @brief      	重置接收消息头部和尾部
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void reset_rx_headTail(void);



/**
 * @func			 	reset_rx_buffer
 * @brief      	清空接收缓冲区
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void reset_rx_buffer(void);



/**
 * @func			 	ESP8266_SendString
 * @brief      	通过串口向ESP8266-01S发送字符串函数
 * @param		  	参数一：char*（要发送的字符串）
 * @retval     	无
 * @note       	无
 */
void ESP8266_SendString(char *str);



/**
 * @func			 	ESP8266_SendAT
 * @brief      	通过串口向ESP8266-01S发送AT指令函数
 * @param		  	参数一：char*（要发送的AT指令）
 * @retval     	无
 * @note       	无
 */
void ESP8266_SendAT(char *at_cmd);



/**
 * @func			 	USART1_Init
 * @brief      	串口1初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void USART1_Init(void);



/**
 * @func			 	ReadLine
 * @brief      	读取一行数据
 * @param		  	参数一：（char*）存储该行数据的空间
								参数二：（uint16_t）最大读取的数据长度
 * @retval     	uint16_t
 * @note       	无
 */
uint16_t ReadLine(char* buffer, uint16_t max_len);
//void Parse_OneNET_Property(const char* json_str);

#endif
