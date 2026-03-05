 /******************************************************************************
 * @file    Water.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/09
 * @brief   水位传感器具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __WATER_H
#define	__WATER_H
#include "stm32f10x.h"
#include "Delay.h"
#include "math.h"
#include "ADC1.h"

#define WATER_READ_TIMES	10  //水位传感器ADC循环读取次数

//水位传感器GPIO宏定义
#define WATER_AO_GPIO_CLK    RCC_APB2Periph_GPIOA	//水位传感器的RCC时钟
#define WATER_AO_GPIO_PORT   GPIOA	//水位传感器的GPIO端口
#define WATER_AO_GPIO_PIN    GPIO_Pin_5	//水位传感器的GPIO引脚
#define ADC_CHANNEL_WATER    ADC_Channel_5  // PA5对应的ADC1通道5



enum state_waterLevel	//卫生间水位状态
{
	STATE_WATER_LEVEL_HIGH = 0,	//水位高
	STATE_WATER_LEVEL_NORMAL,	//水位正常
	STATE_WATER_LEVEL_UNKNOWN	//水位未知
}typedef state_waterLevel;	//将enum state_waterLevel重名为state_waterLevel



/**
 * @func			 	Water_Init
 * @brief      	水位传感器初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void Water_Init(void);



/**
 * @func			 	Water_ADC_Read
 * @brief      	水位传感器检测数据读取函数
 * @param		  	无
 * @retval     	uint16_t
 * @note       	无
 */
uint16_t Water_ADC_Read(void);



/**
 * @func			 	waterLevel_isHigh
 * @brief      	判断卫生间是否积水函数
 * @param		  	无
 * @retval     	state_waterLevel
 * @note       	无
 */
state_waterLevel waterLevel_isHigh(void);

#endif /* __WATER_H */

