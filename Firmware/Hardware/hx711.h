 /******************************************************************************
 * @file    hx711.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/09
 * @brief   称重模块HX711具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __HX711_H
#define	__HX711_H
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Delay.h"


// HX711 GPIO宏定义
#define		HX711_GPIO_CLK									RCC_APB2Periph_GPIOB	//HX711的RCC时钟
#define 	HX711_SCK_GPIO_PORT							GPIOB	//HX711 SCK引脚连接的GPIO端口
#define 	HX711_SCK_GPIO_PIN							GPIO_Pin_11	//HX7111 SCK引脚连接的GPIO引脚
#define 	HX711_DT_GPIO_PORT							GPIOB	//HX711	DT引脚连接的GPIO端口
#define 	HX711_DT_GPIO_PIN								GPIO_Pin_10	//HX711 DT引脚连接的GPIO引脚



//宏定义，将HX711的SCK引脚置高电平
#define HX711_SCK_H				GPIO_SetBits(HX711_SCK_GPIO_PORT,HX711_SCK_GPIO_PIN);



//宏定义，将HX711的SCK引脚置低电平
#define HX711_SCK_L				GPIO_ResetBits(HX711_SCK_GPIO_PORT,HX711_SCK_GPIO_PIN);



//宏定义，读取HX711引脚的数据
#define HX711_DT					GPIO_ReadInputDataBit(HX711_DT_GPIO_PORT, HX711_DT_GPIO_PIN)



/**
 * @func			 	HX711_Init
 * @brief      	HX711初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void HX711_Init(void);



/**
 * @func			 	HX711_GetData
 * @brief      	读取HX711检测数据函数
 * @param		  	无
 * @retval     	unsigned long
 * @note       	无
 */
unsigned long HX711_GetData(void);

#endif /* __ADC_H */

