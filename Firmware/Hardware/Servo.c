 /******************************************************************************
 * @file    Servo.c
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   SG90舵机具体实现代码源文件
 * @version 无
 * @note    无
 ******************************************************************************/



#include "Servo.h"

//门状态标志位
//1表示由外向内关
//2表示由外向内开
//3表示由内向外关
//4表示由内向外开
uint8_t status_door = 1;

float angle_this = 0;//表示舵机当前旋转角度的全局变量

char status_door_str[9];//存储门的状态信息

//舵机初始化函数
void Servo_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			//开启TIM2的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;//定义一个GPIO初始化结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//受外设控制的引脚，均需要配置为复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//引脚选择1号引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度一般选择50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);//将PA1引脚初始化为复用推挽输出	
																	
	TIM_InternalClockConfig(TIM2);		//选择TIM2为时钟源
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 20000 - 1;				//计数周期，即ARR的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;				//预分频器，即PSC的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM2的时基单元
	 
	TIM_OCInitTypeDef TIM_OCInitStructure;//定义结构体变量
	TIM_OCStructInit(&TIM_OCInitStructure);//结构体初始化，若结构体没有完整赋值
	//则最好执行此函数，给结构体所有成员都赋一个默认值
	//避免结构体初值不确定的问题
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//输出比较模式，选择PWM模式1
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性，选择为高，若选择极性为低，则输出高低电平取反
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//输出使能
	TIM_OCInitStructure.TIM_Pulse = 0;//初始的CCR值
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);//将结构体变量交给TIM_OC2Init，配置TIM2的输出比较通道2
	
	TIM_Cmd(TIM2, ENABLE);//使能TIM2，定时器开始运行
	
	angle_this = 90;//初始化默认角度为90度
	
	Servo_SetAngle(angle_this);//设置舵机角度为90度
}

//设置PWM波形的占空比的函数
void PWM_SetCompare2(uint16_t Compare)
{
	TIM_SetCompare2(TIM2, Compare);		//设置CCR2的值
}

//舵机旋转角度设置函数
void Servo_SetAngle(float Angle)
{
	PWM_SetCompare2(Angle / 180 * 2000 + 500);//设置占空比
	//将角度线性变换，对应到舵机要求的占空比范围上
}

//门由外向内开函数
void DoorControl_OutToIn_Open(void)
{
    status_door = 2;//门状态标志位赋值为2表示门由外向内开
	
		uint16_t angle;//存储舵机旋转角度的临时变量
    
    // 从90°以较快的速度旋转到180°
    for(angle = angle_this; angle < 180; angle += 2)
    {
        Servo_SetAngle(angle);//设置舵机旋转角度
        vTaskDelay(20); // 较快转动，延时较短
    }
		
		angle_this = angle;//更新全局变量存储的角度
		
		//status_door =;
}

//门由内向外关（缓慢）
void DoorControl_InToOut_Close(void)
{
    status_door = 3;//门状态信息标志位赋值为3表示门由内向外关
    
		uint16_t angle;////用于存储舵机旋转角度的临时变量
    // 从180°缓慢旋转到90°
   for(angle = angle_this; angle > 90; angle -= 1)
    {
        Servo_SetAngle(angle);//设置舵机旋转角度
        vTaskDelay(40); // 缓慢转动，延时较长
    }
		
		angle_this = angle;//
}

//门由内向外开（缓慢）
void DoorControl_InToOut_Open(void)
{
		status_door = 4;
    
		int16_t angle;
    // 从90°缓慢旋转到0°
    for(angle = angle_this; angle > 0; angle -= 1)//
    {
        Servo_SetAngle(angle);
        vTaskDelay(40); // 缓慢转动，延时较长
    }
    
		angle_this = angle;//更新全局变量存储的角度
}

// 由外向内关门（缓慢）
void DoorControl_OutToIn_Close(void)
{
    status_door = 1;//门状态信息标志位赋值为1表示门由外向内开
    
	uint16_t angle;
    // 从0°缓慢旋转到90°
   for(angle = angle_this; angle < 90; angle += 1)
    {
        Servo_SetAngle(angle);//设置舵机旋转角度
        vTaskDelay(40); // 缓慢转动，延时较长
    }
		
		angle_this = angle;//更新全局变量存储的角度
}

// 获取当前门状态
char* getStatus_door(void)
{
	//char status_door_str[9];在函数中返回了一个 局部变量（local variable）
	//的地址，而这个局部变量在函数返回后就会被销毁，导致返回的指针指向无效内存（野指针）。
	
		switch(status_door)//通过switch循环快速判断门的状态
		{
			case 1://当标志位为1时
				memset(status_door_str, 0, 9);//清空门状态信息存储数组
				strcpy(status_door_str, "closeIn");//将其赋值为closeIn
			break;
			
			case 2://当标志位为2时
				memset(status_door_str, 0, 9);//清空门状态信息存储数组
				strcpy(status_door_str, "openIn");//将其赋值为openIn
			break;
			
			case 3://标志位为3时
				memset(status_door_str, 0, 9);//清空门状态信息存储数组
				strcpy(status_door_str, "closeOut");//将其赋值为closeOut
			break;
			
			case 4://标志位为4时
				memset(status_door_str, 0, 9);//清空门状态信息存储数组
				strcpy(status_door_str, "openOut");//将其赋值为openOut
			break;
		}
    
		return status_door_str;//返回门状态存储数组
}

//向OneNET物联网平台上传门状态值
void Send_Door_Status_To_OneNET(uint16_t id)
{
	//定义一个向ESP8266 01S发送内容的缓冲区
	char cmd[sizeof("AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"door\\\":{\\\"value\\\":\\\"123456789\\\"\\}}}\",0,0")];
    
  switch(status_door)//通过switch循环选择要发送的信息
	{
		case 1://当状态为为1时
			//格式化发送内容：门：向内关
			sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"door\\\":{\\\"value\\\":\\\"closeIn\\\"\\}}}\",0,0", 
                id);
		break;//跳出循环
		
		case 2://当状态为为2时
			//格式化发送内容：门：向内开
			sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"door\\\":{\\\"value\\\":\\\"openIn\\\"\\}}}\",0,0", 
                id);
		break;//跳出循环
		
		case 3://当状态为为3时
			//格式化发送内容：门：向外关
			sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"door\\\":{\\\"value\\\":\\\"closeOut\\\"\\}}}\",0,0", 
                id);
		break;//跳出循环
		
		case 4://当状态为为4时
			//格式化发送内容：门：向外开
			sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"door\\\":{\\\"value\\\":\\\"openOut\\\"\\}}}\",0,0", 
                id);
		break;//跳出循环
	}
    
  ESP8266_SendAT(cmd);//向ESP8266 01S发送数据
}
