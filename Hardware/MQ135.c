 /******************************************************************************
 * @file    MQ135.c
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/09
 * @brief   空气质量传感器MQ-135具体实现代码源文件
 * @version 无
 * @note    无
 ******************************************************************************/



#include "MQ135.h"



//MQ-135初始化函数
void MQ135_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	//定义一个GPIO初始化结构体，用于存储各种初始化参数
		
	RCC_APB2PeriphClockCmd (MQ135_AO_GPIO_CLK, ENABLE );	// 打开 ADC IO端口时钟
	GPIO_InitStructure.GPIO_Pin = MQ135_AO_GPIO_PIN;					// 配置 ADC IO 引脚模式
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		// 设置为模拟输入
	
	GPIO_Init(MQ135_AO_GPIO_PORT, &GPIO_InitStructure);				// 初始化 ADC IO
}



//通过ADC转换读取MQ135的模拟值
uint16_t MQ135_ADC_Read(void) {
    return ADC1_ReadChannel(4);  //PA4对应ADC1的通道4
}



//取十次MQ-135的转换值的平均值并将其返回
uint16_t MQ135_GetData(void)
{
	uint32_t  tempData = 0;	//用于存储MQ-135转换值的临时变量
	
	for (uint8_t i = 0; i < MQ135_READ_TIMES; i++)//通过for循环，取十次MQ-135转换值
	{
		tempData += MQ135_ADC_Read();	//累加每次MQ-135的转换值
		vTaskDelay(5);//延迟5ms，给ADC转换时间
	}

	tempData /= MQ135_READ_TIMES;	//对累加十次的值除以10取得平均值
	return tempData;	//返回平均值
}


//float MQ135_GetData_PPM(void)
//{
//	float  tempData = 0;
//	

//	for (uint8_t i = 0; i < MQ135_READ_TIMES; i++)
//	{
//		tempData += MQ135_ADC_Read();
//		Delay_ms(5);
//	}
//	tempData /= MQ135_READ_TIMES;
//	
//	float Vol = (tempData*5/4096);
//	float RS = (5-Vol)/(Vol*0.5);
////	Vol是输出电压也就是负载RL的电压，RL=0.5KΩ，RS是传感器的气敏电阻，Vcc是电源电压
////	Vol = [Vcc / (RS + RL)] × RL
////	Vol = (Vcc × RL) / (RS + RL)
////	Vol × (RS + RL) = Vcc × RL
////	Vol × RS + Vol × RL = Vcc × RL
////	Vol × RS = Vcc × RL - Vol × RL
////	Vol × RS = RL × (Vcc - Vol)
////	RS = [RL × (Vcc - Vol)] / Vol
////	通过计算RS，可以得到传感器在当前环境下的实际电阻值，然后通过与洁净空气中的基准电阻R0比较，最终计算出气体浓度PPM值。
//	
//	float R0=6.64;
//	
//	float ppm = pow(11.5428*R0/RS, 0.6549f);
//	
//	return ppm;
//}
