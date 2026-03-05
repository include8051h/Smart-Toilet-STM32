 /******************************************************************************
 * @file    FreeRTOS_demo.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   提供程序在FreeRTOS_demo.c中相关事件标志组、互斥锁、任务声明
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __FREERTOS_DEMO__
#define __FREERTOS_DEMO__

#include "FreeRTOS.h"
#include "task.h"
#include "Servo.h"
#include "Delay.h"
#include "event_groups.h"
#include "Body_HW.h"
#include "OLED.h"
#include "HW.h"
#include "Waterbom.h"
#include "Wind.h"
#include "Light.h"
#include "LED.h"
#include "ADC1.h"
#include "Water.h"
#include "MQ135.h"
#include "hx711.h"
#include "stdio.h"
#include "Delay.h"
#include "Safety.h"
#include "Lock.h"
#include "UART.h"
#include "semphr.h"
#include "uart2.h"
#include "help.h"

enum state_one_in	//卫生间内使用者状态
{
	HAS_NO_COME_IN = 0,	//还没进入卫生间
	HAS_COME_IN,	//已经进入卫生间
	UNKNOWN_ONE_IN //初始状态未知
}typedef state_one_in;//将enum state_one_in重命名为state_one_in



/**
 * @func			 	FreeRTOS_Start
 * @brief      	初始化硬件，创建任务、事件标志组、互斥锁，启动任务调度器
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void FreeRTOS_Start(void);



/**
 * @func			 	task_servo_openToIn
 * @brief      	实现门向内开、向外关功能，实现电磁锁上锁功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_servo_openToIn(void *pvPrameters);



/**
 * @func			 	task_bodyHw
 * @brief      	实现门外人体红外热释电传感器HC-SR501检测人体信号功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_bodyHw(void* pvParameters);



/**
 * @func			 	task_servo_openToOut
 * @brief      	实现门向外开功能，实现电磁锁开锁功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_servo_openToOut(void *pvPrameters);



/**
 * @func			 	task_servo_closeToIn
 * @brief      	实现门向内关功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_servo_closeToIn(void *pvParameters);



/**
 * @func			 	task_waterPump
 * @brief      	实现水泵冲水功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_waterPump(void *pvParameters);



/**
 * @func			 	task_wind_src
 * @brief      	实现卫生间异味气体浓度检测功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_wind_src(void* pvParameters);



/**
 * @func			 	task_wind
 * @brief      	实现卫生间通风功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_wind(void* pvParameters);



/**
 * @func			 	task_safety
 * @brief      	实现卫生间定时器计时功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_safety(void* pvParameters);



/**
 * @func			 	task_light_src
 * @brief      	实现环境光照强度检测功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_light_src(void* pvParameters);



/**
 * @func			 	task_light
 * @brief      	实现卫生间开关灯功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_light(void* pvParameters);



/**
 * @func			 	task_waterLeve
 * @brief      	实现水位高度检测功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_waterLevel(void* pvParameters);



/**
 * @func			 	task_oled
 * @brief      	实现卫生间状态显示功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_oled(void* pvParameters);



/**
 * @func			 	task_weight
 * @brief      	实现卫生间纸巾剩余量检测功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_weight(void* pvParameters);



/**
 * @func			 	task_esp8266_test
 * @brief      	实现本地数据上传和上位机命令接收功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void task_esp8266_test(void* pvParameters);



//void task_lightSenor(void* pvParameters);



/**
 * @func			 	vKeyTask
 * @brief      	实现求救按键功能
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void vKeyTask(void *pvParameters);

#endif	//__FREERTOS_DEMO__
