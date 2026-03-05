 /******************************************************************************
 * @file    Safety.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/09
 * @brief   卫生间使用时间定时器具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __SAFETY_H
#define __SAFETY_H

#include "stm32f10x.h"
#include "OLED.h"

enum DoorState//定义一个门状态的枚举类型
{
	DoorState_Opened,//门开
	DoorState_Closed//门关
};



/**
 * @func			 	safety_timer_init
 * @brief      	卫生间使用时间定时器初始化函数
 * @param		  	uint16_t 预设时间
 * @retval     	无
 * @note       	无
 */
void safety_timer_init(uint16_t target_minutes);



/**
 * @func			 	safety_timer_start
 * @brief      	卫生间使用时间定时器开启函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void safety_timer_start(void);



/**
 * @func			 	safety_timer_stop
 * @brief      	卫生间使用时间定时器停止函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void safety_timer_stop(void);



/**
 * @func			 	safety_timer_reset
 * @brief      	卫生间使用时间定时器重置函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void safety_timer_reset(void);



/**
 * @func			 	safety_timer_set_target
 * @brief      	卫生间使用时间定时器修改预设时间函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void safety_timer_set_target(uint16_t target_minutes);



/**
 * @func			 	safety_timer_get_elapsed_sec
 * @brief      	获取已计时秒数函数
 * @param		  	无
 * @retval     	uint32_t
 * @note       	无
 */
uint32_t safety_timer_get_elapsed_sec(void);



/**
 * @func			 	safety_timer_get_elapsed_min
 * @brief      	获取已计时分钟数函数
 * @param		  	无
 * @retval     	uint32_t
 * @note       	无
 */
uint32_t safety_timer_get_elapsed_min(void);
//uint32_t safety_timer_get_remaining(void);



/**
 * @func			 	safety_timer_get_accident_flag
 * @brief      	获取超时标志位函数
 * @param		  	无
 * @retval     	uint8_t
 * @note       	无
 */
uint8_t safety_timer_get_accident_flag(void);



/**
 * @func			 	safety_timer_clear_accident_flag
 * @brief      	清除超时标志位函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void safety_timer_clear_accident_flag(void);
//void display_safety_timer_info(void);

#endif
