 /******************************************************************************
 * @file    BodyHW.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   人体红外热释电传感器HC-SR501具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __BODY_HW_H
#define	__BODY_HW_H
#include "stm32f10x.h"
#include "stdio.h"
#include "UART.h"



#define		BODY_HW_GPIO_CLK								RCC_APB2Periph_GPIOB//人体红外热释电传感器HC-SR501 RCC时钟
#define 	BODY_HW_GPIO_PORT								GPIOB//HC-SR501连接GPIOB端口
#define 	BODY_HW_GPIO_PIN								GPIO_Pin_12 | GPIO_Pin_15//分别连接第12号和15号引脚		



/**
 * @func			 	BODY_HW_Init
 * @brief      	人体红外热释电传感器HC-SR501初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void BODY_HW_Init(void);



/**
 * @func			 	BODY_HW_GetData_out
 * @brief      	返回门外HC-SR501检测数据
 * @param		  	无
 * @retval     	uint16_t
 * @note       	无
 */
uint16_t BODY_HW_GetData_out(void);



/**
 * @func			 	BODY_HW_GetData_in
 * @brief      	返回门内HC-SR501检测数据
 * @param		  	无
 * @retval     	uint16_t
 * @note       	无
 */
uint16_t BODY_HW_GetData_in(void);



/**
 * @func			 	getStatus_user
 * @brief      	返回使用者状态
 * @param		  	无
 * @retval     	uint8_t
 * @note       	无
 */
uint8_t getStatus_user(void);



/**
 * @func			 	Send_User_Status_To_OneNE
 * @brief      	将使用者状态发送给ESO8266 01S
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Send_User_Status_To_OneNET(uint16_t id);

#endif /* __ADC_H */

