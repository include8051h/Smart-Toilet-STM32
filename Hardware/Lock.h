 /******************************************************************************
 * @file    Lock.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/13
 * @brief   门锁功能具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __LOCK_H
#define __LOCK_H

#include "stm32f10x.h"                  // Device header
#include "stdio.h"
#include "UART.h"

#define Lock_RCC RCC_APB2Periph_GPIOA	//门锁对应的RCC时钟
#define Lock_Port GPIOA	//门锁连接的GPIO端口
#define Lock_Pin GPIO_Pin_8	//门锁连接的GPIO引脚
#define ON 1	//开
#define OFF 0	//关	//函数名不可与宏定义重叠



/**
 * @func			 	Lock_Init
 * @brief      	门锁初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Lock_Init(void);



/**
 * @func			 	Lock_OFF
 * @brief      	开锁功能具体实现函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Lock_OFF(void);



/**
 * @func			 	Lock_ON
 * @brief      	上锁功能具体实现函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Lock_ON(void);



/**
 * @func			 	getStatus_lock
 * @brief      	获取门锁状态具体实现函数
 * @param		  	无
 * @retval     	uint8_t
 * @note       	无
 */
uint8_t getStatus_lock(void);



/**
 * @func			 	Send_Lock_Status_To_OneNET
 * @brief      	上传门锁状态具体实现函数
 * @param		  	uint16_t id（发送MQTT数据的id号）
 * @retval     	无
 * @note       	无
 */
void Send_Lock_Status_To_OneNET(uint16_t id);

#endif
