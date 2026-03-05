 /******************************************************************************
 * @file    Light.c
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   光敏电阻传感器驱动具体实现代码源文件
 * @version 无
 * @note    无
 ******************************************************************************/



#include "Light.h"

//uint16_t light_threshold = Light_Threshold_Default;

//光敏电阻传感器初始化
void Light_Init(void)
{
	//开启时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//开启GPIOA的RCC外设时钟
	
	//配置GPIO
	GPIO_InitTypeDef GPIO_InitStructure;	//GPIO初始化结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//选用模拟输出	
	GPIO_InitStructure.GPIO_Pin = Light_Pin;//选中引脚0
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//GPIO速度一般选择50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA端口
	
	//ADC1_Init();
}

//uint16_t Light_GetThrehold(uint16_t threhold)
//{
//	return light_threshold = threhold;
//}

//判断环境光照强度
uint8_t Light_IsDark(void)
{
	//获取光敏电阻传感器检测的数据
	uint16_t light_value = ADC1_ReadChannel(0);
	
	//返回判断结果
	return (light_value < 3400) ? 0 : 1;//返回0表示亮，返回1表示暗
}
