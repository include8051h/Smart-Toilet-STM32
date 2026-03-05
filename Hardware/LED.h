 /******************************************************************************
 * @file    LED.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   LED驱动具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"                  // Device header
#include "stdio.h"
#include "UART.h"
#include "Light.h"

#define LED_Port GPIOB//LED连接的GPIO端口
#define LED_Pin GPIO_Pin_0//LED连接的GPIO引脚
#define ON 1//开
#define OFF 0//关 //函数名不可与宏定义重叠



/**
 * @func			 	Wind_Init
 * @brief      	风扇初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void LED_Init(void);



/**
 * @func			 	LED_ON
 * @brief      	开灯函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void LED_ON(void);



/**
 * @func			 	LED_OFF
 * @brief      	关灯函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void LED_OFF(void);



/**
 * @func			 	getStatus_led
 * @brief      	获取LED状态信息
 * @param		  	无
 * @retval     	uint8_t
 * @note       	无
 */
uint8_t getStatus_led(void);



/**
 * @func			 	Send_Led_Status_To_OneNET
 * @brief      	将LED的状态信息发送给ESP8266 01S
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Send_Led_Status_To_OneNET(void);

#endif
