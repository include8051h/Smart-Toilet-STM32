 /******************************************************************************
 * @file    ADC1.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   ADC具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __ADC1_H
#define __ADC1_H

#include "stm32f10x.h"                  // Device header



/**
 * @func			 	ADC1_Init
 * @brief      	ADC初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void ADC1_Init(void);



/**
 * @func			 	ADC1_ReadChannel
 * @brief      	返回转换后的ADC值
 * @param		  	uint16_t ADC转换通道
 * @retval     	uint16_t
 * @note       	无
 */
uint16_t ADC1_ReadChannel(uint16_t channel);


#endif
