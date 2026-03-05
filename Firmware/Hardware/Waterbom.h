 /******************************************************************************
 * @file    Waterbom.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   微型水泵驱动具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __WATERBOM_H
#define __WATERBOM_H

#include "stm32f10x.h" // Device header
#include <stdbool.h>	//使用bool类型必须引用的头文件
#include <string.h>
#include "UART.h"
#include "stdio.h"

#define Waterbom_RCC RCC_APB2Periph_GPIOA	//水泵RCC时钟
#define Waterbom_Port GPIOA	//水泵连接的GPIO端口
#define Waterbom_Pin GPIO_Pin_7	//水泵连接的GPIO引脚
#define ON 1	//开
#define OFF 0//关 //函数名不可与宏定义重叠



/**
 * @func			 	Waterbom_Init
 * @brief      	水泵初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Waterbom_Init(void);



/**
 * @func			 	Waterbom_OFF
 * @brief      	水泵关函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Waterbom_OFF(void);
//void LED_ON(void);



/**
 * @func			 	Waterbom_ON
 * @brief      	水泵开函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Waterbom_ON(void);
//void LED_OFF(void);



/**
 * @func			 	getStatus_waterPump
 * @brief      	返回水泵状态信息函数
 * @param		  	无
 * @retval     	uint8_t
 * @note       	无
 */
uint8_t getStatus_waterPump(void);



/**
 * @func			 	Send_WaterPump_Status_To_OneNET
 * @brief      	向ESP8266 01S发送水泵状态信息函数
 * @param		  	uint16_t类型 发送MQTT数据使用的id号
 * @retval     	无
 * @note       	无
 */
void Send_WaterPump_Status_To_OneNET(uint16_t id);

#endif
