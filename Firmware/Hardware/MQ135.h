 /******************************************************************************
 * @file    MQ135.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/09
 * @brief   空气质量传感器MQ-135具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __MQ135_H
#define	__MQ135_H
#include "stm32f10x.h"
#include "Delay.h"
#include "ADC1.h"
#include "math.h"
#include "FreeRTOS.h"
#include "task.h"

#define MQ135_READ_TIMES	10  //MQ-135传感器ADC循环读取次数

//MQ-135 ADC宏定义
#define    ADCx    ADC1	//转换MQ-135的ADC为ADC1
#define    ADC_CLK    RCC_APB2Periph_ADC1	//ADC1的RCC时钟


// MQ-135 GPIO宏定义
#define MQ135_AO_GPIO_CLK   RCC_APB2Periph_GPIOA	//MQ-135的RCC时钟
#define MQ135_AO_GPIO_PORT  GPIOA	//MQ-135的GPIO端口
#define MQ135_AO_GPIO_PIN   GPIO_Pin_4	//MQ-135的GPIO引脚
#define ADC_CHANNEL         ADC_Channel_4  // PA4对应ADC1通道4



//void ADCx_Init(void);
//u16 ADC_GetValue(uint8_t ADC_Channel,uint8_t ADC_SampleTime);



/**
 * @func			 	MQ135_Init
 * @brief      	MQ-135初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void MQ135_Init(void);



/**
 * @func			 	MQ135_ADC_Read
 * @brief      	MQ-135的ADC转换值读取函数
 * @param		  	无
 * @retval     	uint16_t
 * @note       	无
 */
uint16_t MQ135_ADC_Read(void);



/**
 * @func			 	MQ135_GetData
 * @brief      	取十次MQ-135的ADC转换值的平均值
 * @param		  	无
 * @retval     	uint16_t
 * @note       	无
 */
uint16_t MQ135_GetData(void);
//float MQ135_GetData_PPM(void);

#endif /* __ADC_H */

