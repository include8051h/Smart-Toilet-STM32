 /******************************************************************************
 * @file    Servo.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   SG90舵机具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f10x.h"                  // Device header
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "UART.h"

#define Servo_Pin GPIO_Pin_1	//SG90舵机信号线连接GPIO 1号引脚
#define Servo_Port GPIOA	//SG90舵机信号线连接的端口GPIOA

// 门状态枚举值定义
typedef enum {
	OPEN_TO_IN = 0,	//向内开
	CLOSE_TO_OUT = 1,	//向外关
	OPEN_TO_OUT = 2,	//向外开
	CLOSE_TO_IN	= 3//向内关
} doorStatus;//将enum doorStatus重名为doorStatus

//char status_door[sizeof("一二三")];



/**
 * @func			 	Servo_Init
 * @brief      	SG90舵机初始化
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Servo_Init(void);



/**
 * @func			 	PWM_SetCompare2
 * @brief      	设置CCR2值
 * @param		  	参数一：uint16_t类型 compare（要设置的PWM波形的占空比）
 * @retval     	无
 * @note       	无
 */
void PWM_SetCompare2(uint16_t compare);



/**
 * @func			 	Servo_SetAngle
 * @brief      	设置舵机旋转角度
 * @param		  	参数一：float类型 angle（要设置的舵机的旋转角度）
 * @retval     	无
 * @note       	无
 */
void Servo_SetAngle(float angle);



/**
 * @func			 	DoorControl_OutToIn_Open
 * @brief      	舵机门由外向内开具体实现函数，本质是让舵机从90度顺时针旋转至180度
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void DoorControl_OutToIn_Open(void);



/**
 * @func			 	DoorControl_InToOut_Close
 * @brief      	舵机门由内向外关具体实现函数，本质是让舵机从180度逆时针旋转至90度
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void DoorControl_InToOut_Close(void);



/**
 * @func			 	DoorControl_InToOut_Open
 * @brief      	舵机门由内向外开具体实现函数，本质是让舵机从90度逆时针旋转至180度
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void DoorControl_InToOut_Open(void);



/**
 * @func			 	DoorControl_OutToIn_Close
 * @brief      	舵机门由外向内关具体实现函数，本质是让舵机从0度顺时针旋转至90度
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void DoorControl_OutToIn_Close(void);



/**
 * @func			 	getStatus_door
 * @brief      	返回门的当前状态具体实现函数
 * @param		  	无
 * @retval     	char类型指针
 * @note       	无
 */
char* getStatus_door(void);



/**
 * @func			 	Send_Door_Status_To_OneNET
 * @brief      	将门的当前状态发送给ESP8266 01S的具体实现函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Send_Door_Status_To_OneNET(uint16_t id);

#endif
