 /******************************************************************************
 * @file    BodyHW.c
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   人体红外热释电传感器HC-SR501具体实现代码源文件
 * @version 无
 * @note    无
 ******************************************************************************/




#include "body_hw.h"

uint8_t status_user = 0;  // 0=关闭, 1=开启,用于向onenet上传卫生间内是否有人的信息,初始状态为0

//人体红外热释电传感器HC-SR501初始化函数
void BODY_HW_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;//GPIO初始化结构体
		
		RCC_APB2PeriphClockCmd (BODY_HW_GPIO_CLK, ENABLE );	// 打开连接 传感器DO 的单片机引脚端口时钟
		GPIO_InitStructure.GPIO_Pin = BODY_HW_GPIO_PIN;			// 配置连接 传感器DO 的单片机引脚模式
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;			// 设置为下拉输入
		
		GPIO_Init(BODY_HW_GPIO_PORT, &GPIO_InitStructure);// 初始化 
	
}

//返回门外HC-SR501检测数据函数
uint16_t BODY_HW_GetData_out(void)
{
	uint16_t tempData;//存储门外HC-SR501检测数据的临时变量
	tempData = GPIO_ReadInputDataBit(BODY_HW_GPIO_PORT, GPIO_Pin_12);//获取数据
	return tempData;//返回检测到的数据
}

//返回门内HC-SR501检测数据函数
uint16_t BODY_HW_GetData_in(void)
{
	uint16_t tempData;//存储门内HC-SR501检测数据的临时变量
	tempData = GPIO_ReadInputDataBit(BODY_HW_GPIO_PORT, GPIO_Pin_15);//获取数据
	switch(tempData)//通过读取的数据来给使用者状态标志位赋值
	{
		case 1://如果门内HC-SR501输出值为1，表示卫生间内有人
			status_user = 1;//使用者状态标志位置一，表示卫生间内有人
		break;//跳出switch循环
		
		case 0://如果门内HC-SR501输出值为0，表示卫生间内没有人
			status_user = 0;//使用者状态标志位置零，表示卫生间内没有人
		break;//跳出switch循环
	}
	
	return tempData;//返回门内HC-SR501检测数据
}

//用于获取卫生间内是否有人的状态值
uint8_t getStatus_user(void)
{
	return status_user;//返回状态值
}

//发送使用者状态信息给ESP82666 01S的函数
void Send_User_Status_To_OneNET(uint16_t id) 
{
	//定义一个发送缓冲区
    char cmd[250];
    
	//格式化发送内容
    sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"user\\\":{\\\"value\\\":%s\\}}}\",0,0", 
                id,status_user ? "true" : "false");
    
	//发送格式化内容
    ESP8266_SendAT(cmd);
}
