 /******************************************************************************
 * @file    Wind.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   DC 5V风扇驱动具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __WIND_H
#define __WIND_H

#include "stm32f10x.h"                  // Device header
#include "stdio.h"
#include "UART.h"

#define Wind_RCC RCC_APB2Periph_GPIOB//风扇的RCC时钟
#define Wind_Port GPIOB//风扇的GPIO端口
#define Wind_Pin GPIO_Pin_1//风扇的GPIO引脚
#define ON 1	//开
#define OFF 0	//关	//函数名不可与宏定义重叠



/**
 * @func			 	Wind_Init
 * @brief      	风扇初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Wind_Init(void);



/**
 * @func			 	Wind_OFF
 * @brief      	关闭风扇函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Wind_OFF(void);
//void LED_ON(void);



/**
 * @func			 	Wind_ON
 * @brief      	开启风扇函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Wind_ON(void);
//void LED_OFF(void);



/**
 * @func			 	getStatus_wind
 * @brief      	返回风扇的状态信息
 * @param		  	无
 * @retval     	uint8
 * @note       	无
 */
uint8_t getStatus_wind(void);



/**
 * @func			 	sendToOnenet_status_wind
 * @brief      	向ESP8266 01S发送风扇的状态信息
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void sendToOnenet_status_wind(void);

#endif
