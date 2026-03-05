 /******************************************************************************
 * @file    hx711.c
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/09
 * @brief   称重模块HX711具体实现代码源文件
 * @version 无
 * @note    无
 ******************************************************************************/



#include "hx711.h"

/**
  * @brief  PM25初始化程序
  * @param  无
  * @retval 无
  */



//HX711初始化函数
void HX711_Init(void)
{
		/*定义一个HX711_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;

		/*开启相关的GPIO外设时钟*/
		RCC_APB2PeriphClockCmd(HX711_GPIO_CLK , ENABLE);
		/*选择要控制的GPIO引脚*/

		//HX711的SCK引脚的GPIO配置
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//配置GPIO模式
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //配置GPIO速度
		GPIO_InitStructure.GPIO_Pin = HX711_SCK_GPIO_PIN;	//配置GPIO引脚
		GPIO_Init(HX711_SCK_GPIO_PORT, &GPIO_InitStructure);	//初始化配置参数
		
		//HX711的DT引脚的GPIO配置
		GPIO_InitStructure.GPIO_Pin = HX711_DT_GPIO_PIN;	//配置GPIO引脚	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//配置GPIO模式   

		GPIO_Init(HX711_DT_GPIO_PORT, &GPIO_InitStructure);	//初始化配置参数	
}



//读取HX711检测数据函数
unsigned long HX711_GetData(void)
{
		unsigned long Count;	//定义一个 32 位变量 Count，用于拼接 HX711 输出的 24 位串行数据。
		unsigned char i;	//循环计数器，用于控制 24 次数据位的读取。
		HX711_SCK_L;	//确保 SCK初始状态为低电平，这是 HX711 通信协议的要求。
	  Delay_us(1);	//短暂延时，确保电平稳定
		Count=0;	//清零 Count，为接收新数据做准备。
	
		//等待 HX711的DT引脚变为低电平。
		//HX711在完成一次内部ADC转换后，会将 DT 拉低，表示数据已准备好，可以读取。
		//如果DT为高，说明转换未完成，必须等待。
		while(HX711_DT);
	
		//HX711 输出 24 位数据，高位在前
		for (i=0;i<24;i++)
	 {
				HX711_SCK_H;//拉高SCK
				Delay_us(1);//短暂延时
				Count=Count<<1;//左移一位，为新数据腾出最低位
				HX711_SCK_L;//拉低SCK
				Delay_us(1);//短暂延时
				if(HX711_DT) Count++;//如果HX711的DT引脚为高电平，Count赋值为1
		}
		HX711_SCK_H;//告诉 HX711 本次读取已完成
		Delay_us(1);//短暂延时
		
		//将 24 位有符号数转换为无符号正数
		Count=Count^0x800000;//最高位取反，其他位不变
	                      //在HX71芯片中，count是一个32位的有符号整数，用于存储称重传感器的读数。
	                      //当count的最高位为1时，表示读数为负数，而HX711芯片不支持负数的读数。
	                      //因此，为了将负数转换为正数，需要将count的最高位取反，即将count与0x800000进行异或操作。
                        //具体来说，0x800000的二进制表示为100000000000000000000000，与count进行异或操作后，
	                      //可以将count的最高位从1变为0，从而得到对应的正数读数。
		HX711_SCK_L;//将 SCK 恢复为低电平，结束本次通信
		Delay_us(1);//短暂延时
		
		return(Count);//返回处理后的 24 位数据
}

