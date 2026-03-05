 /******************************************************************************
 * @file    HW.c
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   红外对管传感器具体实现代码源文件
 * @version 无
 * @note    无
 ******************************************************************************/



#include "HW.h"



//红外对管传感器初始化函数
void HW_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;//GPIO初始化结构体
		
		RCC_APB2PeriphClockCmd (HW_GPIO_CLK, ENABLE );	// 打开连接 传感器DO 的单片机引脚端口时钟
		GPIO_InitStructure.GPIO_Pin = HW_GPIO_PIN_OUT | HW_GPIO_PIN_IN;			// 配置连接 传感器DO 的单片机引脚模式
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			// 设置为上拉输入,因为光电红外传感器的有效输入是低电平
		
		GPIO_Init(HW_GPIO_PORT, &GPIO_InitStructure);				// 初始化 
	
}

//返回门外红外对管传感器检测数据函数
uint16_t HW_GetData_OUT(void)
{
	uint16_t tempData;//存储门外红外对管传感器的临时变量
	tempData = !GPIO_ReadInputDataBit(HW_GPIO_PORT, HW_GPIO_PIN_OUT);//翻转电平，将有效电平从低变为高
	return tempData;//返回检测数据
}

//返回门内红外对管传感器检测数据函数
uint16_t HW_GetData_IN(void)
{
	uint16_t tempData;//存储门内红外对管传感器的临时变量
	tempData = !GPIO_ReadInputDataBit(HW_GPIO_PORT, HW_GPIO_PIN_IN);//翻转电平，将有效电平从低变为高
	return tempData;//返回检测数据
}

