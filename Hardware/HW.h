 /******************************************************************************
 * @file    HW.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   红外对管传感器具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __HW_H
#define	__HW_H
#include "stm32f10x.h"
//#include "adcx.h"
#include "Delay.h"
#include "math.h"

//红外对管传感器GPIO宏定义
#define		HW_GPIO_CLK								RCC_APB2Periph_GPIOB	//红外对管传感器RCC时钟
#define 	HW_GPIO_PORT							GPIOB	//红外对管传感器GPIOB端口
#define 	HW_GPIO_PIN_OUT							GPIO_Pin_13	//门外红外对管传感器输出引脚
#define  	HW_GPIO_PIN_IN							GPIO_Pin_14	//门内红外对管传感器输出引脚



/**
 * @func			 	HW_Init
 * @brief      	红外对管传感器初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void HW_Init(void);



/**
 * @func			 	HW_GetData_OUT
 * @brief      	返回门外红外对管传感器检测数据函数
 * @param		  	无
 * @retval     	uin16_t
 * @note       	无
 */
uint16_t HW_GetData_OUT(void);



/**
 * @func			 	HW_GetData_IN
 * @brief      	返回门内红外对管传感器检测数据函数
 * @param		  	无
 * @retval     	uint16_t
 * @note       	无
 */
uint16_t HW_GetData_IN(void);

#endif /* __ADC_H */

