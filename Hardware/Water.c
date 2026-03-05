 /******************************************************************************
 * @file    Water.c
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   水位传感器具体实现代码源文件
 * @version 无
 * @note    无
 ******************************************************************************/




#include "Water.h"


//水位传感器初始化函数
void Water_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;	//定义一个GPIO初始化结构体，用来存储各种初始化参数
		
		RCC_APB2PeriphClockCmd (WATER_AO_GPIO_CLK, ENABLE );	// 打开 ADC IO端口RCC时钟
		GPIO_InitStructure.GPIO_Pin = WATER_AO_GPIO_PIN;					// 配置 ADC IO 引脚
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		// 设置为模拟输入
		
		GPIO_Init(WATER_AO_GPIO_PORT, &GPIO_InitStructure);				// 初始化 ADC IO

}


//u16 ADC_GetValue(void)
//{
//	//配置ADC通道
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_55Cycles5);
//	
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE); //软件触发ADC转换
//	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET); //读取ADC转换完成标志位
//	return ADC_GetConversionValue(ADC1);
//}



//水位传感器检测数据读取函数
uint16_t Water_ADC_Read(void) {
    return ADC1_ReadChannel(5);  //返回ADC1在通道5转换的值
}



//判断卫生间是否积水函数
state_waterLevel waterLevel_isHigh(void)
{
	uint16_t waterLevel_this = Water_ADC_Read();//存储水位传感器的检测数据
	
	if(waterLevel_this > 1000)//如果检测数据大于1000
	{
		return STATE_WATER_LEVEL_HIGH;//返回水位过高状态值
	}
	else//如果检测数据小于1000
	{
		return STATE_WATER_LEVEL_NORMAL;//返回水位正常状态值
	}
}



