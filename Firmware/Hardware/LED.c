 /******************************************************************************
 * @file    LED.c
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   LED驱动具体实现代码源文件
 * @version 无
 * @note    无
 ******************************************************************************/



#include "LED.h"

uint8_t status_led = 0;  // 0=关闭, 1=开启,用于向onenet上传led的状态,初始状态为false

//LED初始化函数
void LED_Init(void)
{
	//打开RCC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	//配置端口模式
	//1.定义一个GPIO初始化结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//给结构体成员赋值
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//选用推挽输出
//	在STM32中，GPIO可以配置为多种模式，包括推挽输出、开漏输出、模拟输入、浮空输入等。对于驱动LED，通常选择推挽输出模式，原因如下：

//推挽输出（Push-Pull Output）：

//推挽输出结构能够主动输出高电平和低电平。当输出高电平时，P-MOS管导通，将输出拉到VDD（3.3V）；当输出低电平时，N-MOS管导通，将输出拉到GND。

//推挽输出模式提供较强的驱动能力，可以吸收和提供电流，因此可以直接驱动LED等负载。

//开漏输出（Open-Drain Output）：

//开漏输出只能主动拉低电平，不能主动输出高电平。高电平需要外部上拉电阻才能实现。

//开漏输出通常用于总线（如I2C）或者需要电平转换的场合。
	
	GPIO_InitStructure.GPIO_Pin = LED_Pin;//选中B0号
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//GPIO速度一般选择50MHz
	
	GPIO_Init(LED_Port, &GPIO_InitStructure);//初始化
	
	GPIO_SetBits(LED_Port, LED_Pin);//推挽输出初始状态下默认输出低电平
	//因此如果LED一脚接正情况下，LED一上电就会亮起，所以要将单片机输出拉高
}

//开灯函数
void LED_ON(void)
{
	GPIO_ResetBits(LED_Port, LED_Pin);//将LED引脚置为低电平
	status_led = 1;//更新LED状态
}

//关灯函数
void LED_OFF(void)
{
	GPIO_SetBits(LED_Port, LED_Pin);//将LED引脚置为高电平
	status_led = 0;//更新状态
}

//用于获取LED状态值
uint8_t getStatus_led(void)
{
	return status_led;//返回LED的状态值
}

//向ESP8266 01S发送LED状态
void Send_Led_Status_To_OneNET(void) 
{
	//定义一个发送缓冲区
    char cmd[250];
    
	//格式化发送内容：LED：状态
    sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"LED\\\":{\\\"value\\\":%s\\}}}\",0,0", 
                status_led ? "true" : "false");
    
	//将格式化内容发送给ESP8266 01S
    ESP8266_SendAT(cmd);
}
