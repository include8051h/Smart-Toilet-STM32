 /******************************************************************************
 * @file    Light.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   光敏电阻传感器驱动具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/




#ifndef __LIGHT_H
#define __LIGHT_H

#include "stm32f10x.h"                  // Device header
#include "ADC1.h"

// 默认光线阈值（0-4095，值越小表示越暗）
//#define Light_Threshold_Default 2000

#define Light_Port	GPIOA	//光敏电阻传感器连接的GPIO端口
#define Light_Pin	GPIO_Pin_0	//光敏电阻传感器连接的GPIO引脚0



/**
 * @func			 	Light_Init
 * @brief      	光敏电阻传感器初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Light_Init(void);	//光敏电阻传感器初始化
//uint16_t Light_GetValue(void);
//uint16_t Light_GetThrehold(uint16_t threhold);



/**
 * @func			 	Light_IsDark
 * @brief      	判断环境光照强度
 * @param		  	无
 * @retval     	uint8_t
 * @note       	无
 */
uint8_t Light_IsDark(void);	//判断环境光照强度

#endif
