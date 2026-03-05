 /******************************************************************************
 * @file    help.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/13
 * @brief   求救按键功能具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __HELP_H
#define	__HELP_H

#include "stm32f10x.h"                  // Device header
#include "FreeRTOS.h"
#include "queue.h"
#include "LED.h"


//--------------------------------
// 按键状态
//typedef enum {
//    KEY_STATE_IDLE = 0,        // 空闲（等待按下）
//    KEY_STATE_PRESSED,     // 已按下，等待松手
//} KeyState_t;

extern QueueHandle_t xKeyEventQueue; // 用于从ISR传递事件



/**
 * @func			 	Key_EXTI_Init
 * @brief      	求救按键初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Key_EXTI_Init(void);
#endif
