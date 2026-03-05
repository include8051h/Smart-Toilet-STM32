 /******************************************************************************
 * @file    FreeRTOS_demo.c
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/08
 * @brief   创建事件标志组、互斥锁、任务，开启任务调度器
 * @version 无
 * @note    无
 ******************************************************************************/



#include "FreeRTOS_demo.h"
#include "UART.h"

uint16_t id = 4000;	//ESP8266 01S向OneNET传输MQTT数据的id号，不能重复，否则可能会被OneNET误认为重复消息而拒收
//char id_str[4];

uint16_t bodyHw_value_out = 0;	//接收当前人体红外热释电传感器检测值的变量（门外）
uint16_t bodyHw_value_pre_out = 0;	//接收上一次人体红外热释电传感器检测值的变量（门外）

uint8_t firstFlag_softTimer = 1;	//软件定时器softTimer首次开始计时标志位

uint16_t bodyHw_value_in = 0;	//接收当前人体红外热释电传感器检测值的变量（门内）
uint16_t bodyHw_value_pre_in = 0;	//接收上一次人体红外热释电传感器检测值的变量（门内）
state_one_in state_user_in = UNKNOWN_ONE_IN;	//卫生间内使用者状态，初始为未知

state_oled state_oled_this = NOTHING;	//判断OLED屏是否已经写有字符串

char line[256];//用于存储从串口1读取的数据的全局字符数组
uint16_t len;//用于存储从串口1接收到的数据长度

int value;//用于存储HX711称重模块数据
float weight;//用于存储HX711称重模块计算公式的结果
int32_t reset;//用于存储HX711称重模块的空载值，称重模块计算公式参数
//char buff[30];
float Weights=100.0;  //100g，称重模块计算公式参数
int32_t Weights_100=8581584;  //100g，HX711称重模块称量100g砝码所输出的检测值，计算公式参数	

uint8_t state_first_safety = 1;	//用于判断卫生间计时定时器是否已经启动
EventBits_t uxBits;

//事件标志组：卫生间自动开关门控制	参数声明
//第一位：置一：表示开启门外人体红外热释电传感器检测。置零：表示关闭
//第二位：置一：表示开启舵机门向内开和门内外红外光电传感器检测功能包含舵机门向外关。置零：表示关闭
//第三位：置一：舵机门向外开
//第四位：置一：舵机门向内开
//第五位：ESP8266 01S收发数据任务
//第八位：置一：卫生间使用时间计时
//第六位：首次向OneNET上传数据
//第七位：光敏
//第九位：纸巾剩余量
//第十位：水位
//第十一位：OLED显示
//第十二位：风扇
//第十三位：设备控制权限归属。置一表示由STM32自动控制，置零表示由管理员手动控制，默认自动控制
//第十四位：求救按键
EventGroupHandle_t handle_eventGroup_servo_openOrCloseCtrl;



//声明一个FreeRTOS软件定时器
TimerHandle_t handle_softTimer;
void callBackFunc_softTimer(TimerHandle_t xTimer);	//声明softTimer对应的回调函数



//声明一个互斥锁，避免各个任务争抢同一个串口资源
QueueHandle_t handle_semMutex_usart;
//光敏电阻传感器互斥锁
QueueHandle_t handle_semMutex_lighting;
//MQ135气体传感器互斥锁
QueueHandle_t handle_semMutex_mq135;



//第一位任务：人体红外热释电传感器	参数声明
TaskHandle_t handle_task_bodyHw;	//人体红外热释电传感器任务句柄
#define STACK_SIZE_TASK_BODYHW 128	//人体红外热释电传感器任务栈空间
#define PRIORITY_TASK_BODYHW 2	//人体红外热释电传感器任务优先级



//第二位任务：舵机门向内开	参数声明
TaskHandle_t handle_task_servo_openToIn;	//舵机门向内开任务句柄
#define STACK_SIZE_TASK_SERVO_OPEN_TO_IN 128	//舵机门向内开任务栈空间
#define PRIORITY_TASK_SERVO_OPEN_TO_IN 2	//舵机门向内开任务优先级



//第三位任务：舵机门向外开 参数声明
TaskHandle_t handle_task_servo_openToOut;	//舵机门向外关任务句柄
#define STACK_SIZE_TASK_SERVO_OPEN_TO_OUT 128	//舵机门向外关任务栈空间
#define PRIORITY_TASK_SERVO_OPEN_TO_OUT 2	//舵机门向外关任务优先级



//第四位任务：舵机门向内关 参数声明
TaskHandle_t handle_task_servo_closeToIn;	//第四位任务句柄
#define STACK_DEPTH_TASK_SERVO_CLOSE_TO_IN 128	//第四位任务栈空间
#define PRIORITY_TASK_SERVO_CLOSE_TO_IN 2	//第四位任务优先级



//第五位任务：水泵冲水 参数声明
TaskHandle_t handle_task_waterPump;	//水泵冲水任务句柄
#define STACK_DEPTH_TASK_WATER_PUMP 128	//水泵冲水任务栈空间
#define PRIORITY_TASK_WATER_PUMP 2	//水泵冲水任务优先级

//任务：异味气体浓度检测 参数声明
TaskHandle_t handle_task_wind_src;	//异味气体浓度检测任务句柄
#define STACK_DEPTH_TASK_WIND_SRC 128	//异味气体浓度检测任务栈空间
#define PRIORITY_TASK_WIND_SRC 2	//异味气体浓度检测任务优先级

//第六位任务：风扇通风 参数声明
TaskHandle_t handle_task_wind;	//风扇通风任务句柄
#define STACK_DEPTH_TASK_WIND 128	//风扇通风任务栈空间
#define PRIORITY_TASK_WIND 2	//风扇通风任务优先级

//任务：环境光照强度检测 参数声明
TaskHandle_t handle_task_light_src;	//环境光照强度检测任务句柄
#define STACK_DEPTH_TASK_LIGHT_SRC 128 //环境光照强度任务栈空间//256//能用
#define PRIORITY_TASK_LIGHT_SRC 2	//环境光照强度任务优先级

//任务：自动开关灯 参数声明
TaskHandle_t handle_task_light;	//自动开关灯任务句柄
#define STACK_DEPTH_TASK_LIGHT 128 //自动开关灯任务栈空间//256
#define PRIORITY_TASK_LIGHT 2	//自动开关灯任务优先级

//任务：积水检测 参数声明
TaskHandle_t handle_task_waterLevel;	//积水检测任务
#define STACK_DEPTH_TASK_WATER_LEVEL 256	//积水检测任务栈空间//256
#define PRIORITY_TASK_WATER_LEVEL 2	//积水检测任务优先级

//第八位任务：卫生间使用时间计时 参数声明
TaskHandle_t handle_task_safety;	//卫生间使用时间计时任务句柄
#define STACK_DEPTH_TASK_SAFETY 128	//卫生间使用时间计时任务句柄
#define PRIORITY_TASK_SAFETY 2	//卫生间使用时间计时任务优先级

//任务：OLED显示 参数声明
TaskHandle_t handle_task_oled;	//OLED显示任务句柄
#define STACK_DEPTH_TASK_OLED 128	//OLED显示任务栈{空间
#define PRIORITY_TASK_OLED 2	//OLED显示任务优先级

//任务：纸巾剩余量检测 参数声明
TaskHandle_t handle_task_weight;	//纸巾剩余量检测任务句柄
#define STACK_DEPTH_TASK_WEIGHT 256 	//纸巾剩余量检测任务栈空间//512
#define PRIORITY_TASK_WEIGHT 2	//纸巾剩余量检测任务优先级

//任务：ESP8266 01S收发数据 参数声明
TaskHandle_t handle_task_esp8266_test;	//ESP8266 01S收发数据任务句柄
#define STACK_DEPTH_TASK_ESP8266_TEST 384	//ESP8266 01S收发数据任务栈空间//1024
#define PRIORITY_TASK_ESP8266_TEST 2	//ESP8266 01S收发数据任务优先级

//任务：求救按键 参数声明
TaskHandle_t handle_vKeyTask;	//求救按键任务句柄
#define STACK_DEPTH_VKEYTASK 128 	//求救按键任务栈空间//256
#define PRIORITY_VKEYTASK 2	//求救按键任务优先级

//启动FreeRTOS
void FreeRTOS_Start(void)
{
	//硬件初始化
	LED_Init();	//初始化LED
	
	Delay_Init();  // 初始化延时函数
	Servo_Init();	//初始化舵机
	BODY_HW_Init();	//初始化人体红外热释电传感器
	OLED_Init();	//初始化OLED
	Waterbom_Init();	//初始化水泵
	Wind_Init();	//初始化风扇
	Light_Init();	//初始化光敏电阻传感器
	
	ADC1_Init();	//初始化ADC1
	Water_Init(); //初始化水位传感器
	HX711_Init();	//初始化称重模块
	
	reset = HX711_GetData();	//称量空载值
	safety_timer_init(30);	//卫生间使用时间计时定时器初始化，为了方便演示，这里计时45s
	Lock_Init();	//初始化电磁锁
	MQ135_Init();	//初始化空气质量传感器MQ135
	
	USART1_Init();  // 初始化ESP8266串口
	USART2_Init(); // 初始化串口2，用于printf串口打印
	Key_EXTI_Init(); //初始化求救按键

    printf("Hello from USART2!\r\n"); //用于调试，查看程序执行到哪一步
		
	
	
	//创建互斥锁：避免多个任务竞争同一个串口资源
	handle_semMutex_usart = xSemaphoreCreateMutex();
	//创建光敏电阻传感器互斥锁
	handle_semMutex_lighting = xSemaphoreCreateMutex();
	//创建mq135气体传感器互斥锁
	handle_semMutex_mq135 = xSemaphoreCreateMutex();
	
	
	//创建事件标志组：卫生间自动开关门控制
	handle_eventGroup_servo_openOrCloseCtrl = xEventGroupCreate();
	
	//创建定时器：误触自动关门
	handle_softTimer = xTimerCreate("softTimer", 1000*15, pdFALSE, NULL, callBackFunc_softTimer);
	
	
	
	//第一位 创建任务：人体红外传感器
	xTaskCreate(task_bodyHw, "task_bodyHw", STACK_SIZE_TASK_BODYHW, NULL, PRIORITY_TASK_BODYHW, &handle_task_bodyHw);
	
	//第二位 创建任务：舵机向内开（内含向外关功能）
	xTaskCreate(task_servo_openToIn, "task_servo_openToIn", STACK_SIZE_TASK_SERVO_OPEN_TO_IN, NULL, PRIORITY_TASK_SERVO_OPEN_TO_IN, &handle_task_servo_openToIn);
	
	//第三位 创建任务：舵机向外开
	xTaskCreate(task_servo_openToOut, "task_servo_openToOut", STACK_SIZE_TASK_SERVO_OPEN_TO_OUT, NULL, PRIORITY_TASK_SERVO_OPEN_TO_OUT, &handle_task_servo_openToOut);
	
	//第四位 创建任务：舵机门向内关
	xTaskCreate(task_servo_closeToIn, "task_servo_closeToIn", STACK_DEPTH_TASK_SERVO_CLOSE_TO_IN,	NULL,	PRIORITY_TASK_SERVO_CLOSE_TO_IN, &handle_task_servo_closeToIn);
	
	//第五位 创建任务：水泵冲水
	xTaskCreate(task_waterPump, "task_waterPump", STACK_DEPTH_TASK_WATER_PUMP, NULL, PRIORITY_TASK_WATER_PUMP, &handle_task_waterPump);
	
	//创建任务：检测异味气体浓度
	xTaskCreate(task_wind_src,	"task_wind_src", STACK_DEPTH_TASK_WIND_SRC, NULL, PRIORITY_TASK_WIND_SRC, &handle_task_wind_src);

	//第六位 创建任务：风扇通风
	xTaskCreate(task_wind, "task_wind", STACK_DEPTH_TASK_WIND, NULL, PRIORITY_TASK_WIND, &handle_task_wind);
	
	//第七位 创建任务：卫生间使用时间计时
	xTaskCreate(task_safety, "task_safety", STACK_DEPTH_TASK_SAFETY, NULL, PRIORITY_TASK_SAFETY, &handle_task_safety);
	
	//创建任务：检测环境亮度
	xTaskCreate(task_light_src,	"task_light_src", STACK_DEPTH_TASK_LIGHT_SRC, NULL, PRIORITY_TASK_LIGHT_SRC, &handle_task_light_src);
	
	//创建任务：自动开关灯
	xTaskCreate(task_light, "task_light", STACK_DEPTH_TASK_LIGHT, NULL, PRIORITY_TASK_LIGHT, &handle_task_light);
	
	//创建任务：检测水位高度
	xTaskCreate(task_waterLevel, "task_waterLevel", STACK_DEPTH_TASK_WATER_LEVEL, NULL, PRIORITY_TASK_WATER_LEVEL, &handle_task_waterLevel);
	
	//创建任务：OLED显示
	xTaskCreate(task_oled, "task_oled", STACK_DEPTH_TASK_OLED, NULL, PRIORITY_TASK_OLED, &handle_task_oled);
	
	//创建任务：纸巾剩余量检测
	xTaskCreate(task_weight, "task_weight", STACK_DEPTH_TASK_WEIGHT, NULL, PRIORITY_TASK_WEIGHT, &handle_task_weight);
	
	//创建任务：ESP8266 01S 收发数据
	xTaskCreate(task_esp8266_test, "task_esp8266_test", STACK_DEPTH_TASK_ESP8266_TEST, NULL, PRIORITY_TASK_ESP8266_TEST, &handle_task_esp8266_test);

	//创建任务：求救按键
	xTaskCreate(vKeyTask, "KeyTask", STACK_DEPTH_VKEYTASK, NULL, PRIORITY_VKEYTASK, &handle_vKeyTask);

	//开启事件标志组：首次向OneNET上传数据
	xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<5);

	//启动调度器
	vTaskStartScheduler();
}


//ESP8266 01S收发数据任务
void task_esp8266_test(void* pvParameters) 
{
	printf("Hello from USART9!\r\n"); //用于调试，方便查看程序执行到哪一步
	
	//等待系统全面初始化完成后才可以开始
	xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<5, pdFALSE, pdFALSE, portMAX_DELAY);
		
	uint8_t this_status_lock = 9;	//电磁锁的状态标志位
		
  vTaskDelay(2000);// 先等待2秒让系统稳定
		
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
  ESP8266_SendAT("AT+RST"); //向ESP8266 01S发送重启指令
	xSemaphoreGive(handle_semMutex_usart);//释放串口1互斥信号量
	
	while(1)//循环判断指令是否发送成功
	{
		ReadLine(line, sizeof(line));	//从串口1中读取数据
											
		printf("%s\n", line);	//打印读取到的数据
		
		if (strstr(line, "ready") || strstr(line, "#"))	//如果读取到的数据含有ready或#字符，表示指令发送成功
		{
				break;	//退出while循环
		}
		else if(strstr(line, "ERROR"))//如果指令发送失败
		{
				xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
				ESP8266_SendAT("AT+RST");  //重新发送指令
				xSemaphoreGive(handle_semMutex_usart);	//释放串口1互斥信号量
//				id++;	//id号加1，防止重复
		}
		else	//如果什么都没有读取到
		{
			vTaskDelay(1000);	//延迟1秒后再次读取
		}	
	}
										
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
  ESP8266_SendAT("ATE0"); //发送关闭ESP8266 01S回显功能的指令
	xSemaphoreGive(handle_semMutex_usart);//释放串口1互斥信号量
	
	while(1)//循环判断指令是否发送成功
	{
		ReadLine(line, sizeof(line));//从串口1中读取数据
											
		printf("%s\n", line);	//打印读取到的数据
		
		if (strstr(line, "ATE0") || strstr(line, "OK"))//如果ESP8266 01S回复ATE0或OK，表示指令发送成功
		{
			break;//退出循环
		}
		
		else if(strstr(line, "ERROR"))//如果ESP8266 01S回复ERROR
		{
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
			ESP8266_SendAT("ATE0"); //重新发送指令
			xSemaphoreGive(handle_semMutex_usart);//释放串口1互斥信号量
//			id++;
		}
		
		else//如果ESP8266 01S没有回复
		{
			vTaskDelay(1000);//延迟1秒后再次读取
		}	
	}
	
        
        
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
	ESP8266_SendAT("AT+CWMODE=1\r\n");//向ESP8266 01S发送设置STA模式的指令
	xSemaphoreGive(handle_semMutex_usart);//释放串口1互斥信号量
	
	while(1)//循环判断指令是否发送成功
	{
		ReadLine(line, sizeof(line));//读取串口1数据
								
		printf("%s\n", line);//打印读取到的数据
		
		if (strstr(line, "AT+CWMODE=1") || strstr(line, "OK"))//如果ESP8266 01S回复"AT+CWMODE=1"或"OK"
		{
			break;//退出判断循环
		}
		
		else if(strstr(line, "ERROR"))//如果ESP8266 01S回复ERROR
		{
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
			ESP8266_SendAT("AT+CWMODE=1\r\n");//重新发送指令
			xSemaphoreGive(handle_semMutex_usart);//释放串口1互斥信号量
//			id++;//id+1，防止重复
		}
		
		else//如果ES[8266 01S没有回复
		{
			vTaskDelay(1000);//延迟1秒后再次读取
		}	
	}
				
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
  ESP8266_SendString("AT+CWJAP=\"ldn64\",\"15359329728\"\r\n");//时ESP8266 01S连接WIFI//Chuang  lzq7860810  
	xSemaphoreGive(handle_semMutex_usart);//释放串口1互斥信号量
	
	while(1)//循环判断指令是否发送成功
	{
		ReadLine(line, sizeof(line));//从串口1读取数据
								
		printf("%s\n", line);//打印读取到的数据
		
		if (strstr(line, "OK"))//如果ESP8266 01S回复OK
		{
			break;//退出判断循环
		}
		
		else if(strstr(line, "ERROR"))//如果ESP8266 01S回复ERROR
		{
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
			ESP8266_SendString("AT+CWJAP=\"Chuang\",\"lzq7860810\"\r\n");//重新发送指令//LaptopLin  123456789 
			xSemaphoreGive(handle_semMutex_usart);//释放串口1互斥信号量
//			id++;//id加1，防止重复
		}
			
		else//如果ESP8266 01S没有回复
		{
			vTaskDelay(1000);//延迟1秒再次读取
		}	
	}											
				
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
	//配置ESP8266模块连接OneNET IoT平台的MQTT认证参数
	ESP8266_SendString("AT+MQTTUSERCFG=0,1,\"upper_computer\",\"r7vg0Nn75e\","
                   "\"version=2018-10-31&res=products%2Fr7vg0Nn75e%2Fdevices%2Fupper_computer"
                   "&et=1827627080&method=md5&sign=6vOq3cBDqukXtL2m%2FHod%2Bg%3D%3D\",0,0,\"\"\r\n");
	xSemaphoreGive(handle_semMutex_usart);//释放串口1互斥信号量
	
	while(1)//循环判断指令是否发送成功
	{
		ReadLine(line, sizeof(line));//从串口1读取数据
								
		printf("%s\n", line);//打印读取到的数据
		
		//如果ESP8266 01S回复OK或其回复内容包含"%3D"
		if (strstr(line, "OK") || strstr(line, "%3D")) 
		{
			break;//退出判断循环
		}
											
		else if(strstr(line, "ERROR"))//如果ESP8266 01S回复ERROR
		{
			printf("this guy\r\n");//调试信息，用于判断程序执行的位置
			
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
			//重新发送指令
			ESP8266_SendString("AT+MQTTUSERCFG=0,1,\"upper_computer\",\"r7vg0Nn75e\","
      "\"version=2018-10-31&res=products%2Fr7vg0Nn75e%2Fdevices%2Fupper_computer"
      "&et=1827627080&method=md5&sign=6vOq3cBDqukXtL2m%2FHod%2Bg%3D%3D\",0,0,\"\"\r\n");
			//释放串口1互斥信号量
			xSemaphoreGive(handle_semMutex_usart);
//			id++;//id加1，防止重复
		}
		
		else//如果ESP8266 01S没有回复
		{
			vTaskDelay(1000);//延迟1秒再次读取
		}	
	}
	
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
	ESP8266_SendString("AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,0\r\n");//尝试连接OneNET物联网平台的MQTT服务器
	xSemaphoreGive(handle_semMutex_usart);//释放串口1互斥信号量
	
	while(1)//循环判断指令是否发送成功
	{
		ReadLine(line, sizeof(line));//从串口1读取数据
								
		printf("%s\n", line);//打印读取到的数据
		
		if (strstr(line, "OK"))//如果EPS8266 01S回复OK 
		{
			break;//退出判断循环
		}
		
		else if(strstr(line, "ERROR"))//如果ESP8266 01S回复ERROR
		{
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
			ESP8266_SendString("AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,0\r\n");//尝试连接OneNET物联网平台的MQTT服务器
			xSemaphoreGive(handle_semMutex_usart);//释放串口1互斥信号量
//			id++;
		}
		
		else//如果ESP8266 01S没有回复
		{
			vTaskDelay(1000);//延迟1秒再次读取
		}	
	}
				
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
	//订阅向物联网平台上传数据的主题
	ESP8266_SendString("AT+MQTTSUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\",0\r\n");
	xSemaphoreGive(handle_semMutex_usart);//释放串口1互斥信号量
	
	while(1)//循环判断指令是否发送成功
	{
		ReadLine(line, sizeof(line));//从串口1读取数据
								
		printf("%s\n", line);//打印读取到的数据
		
		if (strstr(line, "OK")) //如果ESP8266 01S回复OK
		{
			break;//退出判断循环
		}
		
		else if(strstr(line, "ERROR"))//如果ESP8266 01S回复ERROR
		{
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
			//订阅向物联网平台上传数据的主题
			ESP8266_SendString("AT+MQTTSUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\",0\r\n");
			xSemaphoreGive(handle_semMutex_usart);//释放串口1互斥信号量
//			id++;
		}
		
		else//ESP8266 01S没有回复
		{
			vTaskDelay(1000);//延迟1秒后再次读取
		}	
	}				
				
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
	//订阅从物联网平台接收数据的主题
	ESP8266_SendString("AT+MQTTSUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/set\",0\r\n");
	xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				
	while(1)//循环判断指令是否发送成功
	{
		ReadLine(line, sizeof(line));//从串口1读取数据
								
		printf("%s\n", line);//打印读取到的数据
		
		if (strstr(line, "OK"))//如果ESP8266 01S回复OK 
		{
			break;//退出判断循环
		}
		
		else if(strstr(line, "ERROR"))//如果ESP8266 01S回复ERROR
		{
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
			////订阅从物联网平台接收数据的主题
			ESP8266_SendString("AT+MQTTSUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/set\",0\r\n");
			xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
//			id++;
		}
		
		else//如果ESP8266 01S没有回复
		{
			vTaskDelay(1000);//延迟1秒后再次读取
		}	
	}
				
	char cmd[250];//发送内容缓冲区

	//格式化发送内容：水泵、风扇、LED的状态信息
	sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
          "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"waterPump\\\":{\\\"value\\\":%s\\}\\,\\\"wind\\\":{\\\"value\\\":%s\\}\\,"
					"\\\"LED\\\":{\\\"value\\\":%s\\}}}\","
					"0,0",
					id,getStatus_waterPump() ? "true" : "false", getStatus_wind() ? "true":"false",
					getStatus_led() ? "true" : "false");			 
				 
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
	ESP8266_SendAT(cmd);//将内容发送给ESP8266 01S
	xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
	id++;//id加一，防止重复
	
	while(1)//循环判断内容是否发送成功
	{
		ReadLine(line, sizeof(line));//读取从串口1读取到的数据
								
		printf("%s\n", line);//打印读取到的数据
		
		if (strstr(line, "OK"))//如果发送成功 
		{
			break;//退出判断循环
		}
		
		else if(strstr(line, "ERROR"))//如果发送失败
		{
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
			ESP8266_SendAT(cmd);//重新发送
			xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
			id++;//id加一，防止重复
		}
		
		else//如果ESP8266 01S没有回复
		{
			vTaskDelay(1000);//延迟1秒再次读取
		}	
	}			

	memset(cmd, 0, 250);//清空发送缓冲区
	
	//格式化发送内容：电磁锁的状态信息，卫生间内是否有人	
	sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
          "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"lock\\\":{\\\"value\\\":%s\\}\\,"
					"\\\"user\\\":{\\\"value\\\":%s\\}\\,\\\"door\\\":{\\\"value\\\":\\\"closeIn\\\"\\}}}\","
					"0,0",
					id,getStatus_lock() ? "true":"false", getStatus_user() ? "true" : "false");
				
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
	ESP8266_SendAT(cmd);//发送内容给ESP8266 01S
	xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量	
	id++;//id加一，防止重复
				
	while(1)//循环判断内容是否发送成功
	{
		ReadLine(line, sizeof(line));//从串口1读取数据
								
		printf("%s\n", line);//打印读取到的数据
		
		if (strstr(line, "OK"))//如果发送成功 
		{
			break;//退出判断循环
		}
		
		else if(strstr(line, "ERROR"))//如果发送失败
		{
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
			ESP8266_SendAT(cmd);//重新发送内容
			xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
			id++;//id加一，防止重复
		}
		
		else//如果ESP8266 01S没有回复
		{
			vTaskDelay(1000);//延迟1秒再次读取
		}	
	}
	
	//定义一个发送内容缓冲区
	char cmd3[sizeof("AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
          "\"{\\\"id\\\":123456\\,\\\"params\\\":{\\\"lighting\\\":{\\\"value\\\":1234567\\}\\,"
					"\\\"air\\\":{\\\"value\\\":1234567\\}}}\","
					"0,0")];
	
	//格式化发送内容：环境光照强度：亮，异味气体浓度：正常
	sprintf(cmd3, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
          "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"lighting\\\":{\\\"value\\\":\\\"bright\\\"\\}\\,"
					"\\\"air\\\":{\\\"value\\\":\\\"normal\\\"\\}}}\","
					"0,0",
					id);
				
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
	ESP8266_SendAT(cmd3);//向ESP8266 01S发送内容
	xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
	id++;//id加一，防止重复
	
	while(1)//循环判断内容是否发送成功
	{
		ReadLine(line, sizeof(line));//读取从串口1接收到的内容
		
		printf("%s\n", line);//打印接收到的内容
		
		if (strstr(line, "OK"))//如果ESP8266 01S回复OK
		{
			break;//退出
		}
		
		else if(strstr(line, "ERROR"))//如果ESP8266 01S回复ERROR
		{
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
			ESP8266_SendAT(cmd3);//发送内容
			xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
			id++;//id加一，防止重复
		}
		
		else//如果ESP8266 01S没有回复
		{
			vTaskDelay(1000);//延时1秒再次读取
		}	
	}
	
	//定义一个发送内容缓冲区
	char cmd4[sizeof("AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
          "\"{\\\"id\\\":123456\\,\\\"params\\\":{\\\"method\\\":{\\\"value\\\":\\\"1234567\\\"\\}\\,"
					"\\\"timeOut\\\":{\\\"value\\\":123456\\}\\,\\\"reset\\\":{\\\"value\\\":false\\}}}\","
					"0,0")+5];
	
	
	//格式化发送内容：系统控制方式：自动，卫生间使用时间是否超时，重置：已重置
	sprintf(cmd4, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
          "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"method\\\":{\\\"value\\\":\\\"auto\\\"\\}\\,"
					"\\\"timeOut\\\":{\\\"value\\\":%s\\}\\,\\\"reset\\\":{\\\"value\\\":false\\}}}\","
					"0,0",
	id,safety_timer_get_accident_flag() ? "true" : "false");
	
	
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
	ESP8266_SendAT(cmd4);//发送内容给ESP8266 01S
	xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
	id++;//id加一，防止重复
	
	
	while(1)//循环判断内容是否发送成功
	{
		ReadLine(line, sizeof(line));//从串口1读取数据
		
		printf("%s\n", line);//打印读取到的数据
		
		if (strstr(line, "OK"))//如果ESP8266 01S回复OK 
		{
			break;//跳出判断循环
		}
		
		else if(strstr(line, "ERROR"))//如果ESP8266 01S回复ERROR
		{
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
			ESP8266_SendAT(cmd4);//将内容发送给ESP8266 01S
			xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
			id++;//id加一，防止重复
		}
		
		else//如果ESP8266 01S没有回复
		{
			vTaskDelay(1000);//延时1秒后再次读取
		}	
	}
							
	memset(cmd4, 0, sizeof(cmd4));//清空发送内容缓冲区
	
	//格式化发送内容：求救信号：无
	sprintf(cmd4, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
          "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"help\\\":{\\\"value\\\":false\\}"
					"}}\","
					"0,0",id);
	
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
	ESP8266_SendAT(cmd4);//将内容发送给ESP8266 01S
	xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
	id++;//id加一，防止重复
	
	while(1)
	{
		ReadLine(line, sizeof(line));//从串口1读取到数据
								
		printf("%s\n", line);//打印读取到的数据
		
		if (strstr(line, "OK"))//如果ESP8266 01S回复OK
		{
			break;//退出判断循环
		}
		
		else if(strstr(line, "ERROR"))//如果EPS8266 01S回复ERROR
		{
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1互斥信号量
			ESP8266_SendAT(cmd4);//将内容发送给ESP8266 01S
			xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
			id++;//id加一，防止重复
		}
		
		else//如果ESP8266 01S没有回复
		{
			vTaskDelay(1000);//延时1秒再次读取
		}	
	}
	
	//LED亮三秒后熄灭表示系统已将本地执行模块的状态信息和传感器检测到的数据上传至云平台
//	LED_ON();			
//	vTaskDelay(3000);
//	LED_OFF();
	
	//开启事件标志位
	xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<0);	//门外人体热释电传感器
	xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<6);	//光敏
	xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<8);	//纸巾剩余量
	xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<9);	//水位
	xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<10);	//OLED显示
	xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<11);	//风扇
	xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<12);	//默认自动控制

	reset_rx_headTail();//重置串口1接收缓冲区头尾位置
	reset_rx_buffer();//清空串口1接收缓冲区
	
  while(1) 
	{			 
		len = ReadLine(line, sizeof(line));//获取从串口1接收到的数据长度
		
		if (len > 5)//如果数据长度超过5
		{
			printf("%s\n", line);	//打印接收到的数据
			
			if(strstr(line, "wind"))//如果读取的数据内容含有wind
			{
				if(strstr(line,"true"))//如果读取的数据内容含有true
				{
					Wind_ON();//打开风扇
									
					memset(cmd, 0, sizeof(cmd));//清空串口1发送缓冲区
					
					//格式化发送内容：风扇：开
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"wind\\\":{\\\"value\\\":%s\\}}}\","
					"0,0",
					id,getStatus_wind() ? "true" : "false");
					
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止发送过于频繁云平台接收不到
					ESP8266_SendAT(cmd);//发送内容	
					id++;//id加一防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					memset(cmd, 0, sizeof(cmd));//清空串口1的发送缓冲区
					
					//格式化发送内容：命令下传：风扇开
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdWindOn\\\"\\}}}\","
					"0,0",
					id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，避免过于频繁发送
					ESP8266_SendAT(cmd);//发送内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
				
				else if(strstr(line,"false"))//如果接收内容中含有false
				{
					Wind_OFF();//关闭风扇
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：风扇：关
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"wind\\\":{\\\"value\\\":%s\\}}}\","
					"0,0",
					id,getStatus_wind() ? "true" : "false");
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，避免发送过于频繁
					ESP8266_SendAT(cmd);//发送内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：命令下传：风扇关
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdWindOff\\\"\\}}}\","
					"0,0",id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒避免发送过于频繁
					ESP8266_SendAT(cmd);//发送内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
				
				else//如果只含有wind而不包含其他
				{
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					
					//格式化发送内容：命令下传：风扇命令失败
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdFanFailed\\\"\\}}}\","
					"0,0",id);
					
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，避免发送过于频繁
					ESP8266_SendAT(cmd);//发送内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
			}
							
			else if(strstr(line, "LED"))//如果接收的内容包含LED
			{
				if(strstr(line,"true"))//如果接收的内容包含true
				{
					LED_ON();//开灯
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：LED：开
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"LED\\\":{\\\"value\\\":%s\\}}}\","
					"0,0",id,getStatus_led() ? "true" : "false");
					
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，避免发送过于频繁
					ESP8266_SendAT(cmd);//发送内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口的互斥信号量
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：命令下传：开灯
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdLedOn\\\"\\}}}\","
					"0,0",id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止发送过于频繁
					ESP8266_SendAT(cmd);//发送内容	
					id++;//id加一，防止发送过于频繁				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
				else if(strstr(line,"false"))//如果接收的内容包含false
				{
					LED_OFF();//关灯
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：关灯
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"LED\\\":{\\\"value\\\":%s\\}}}\","
					"0,0",id,getStatus_led() ? "true" : "false");
					
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止发送过于频繁
					ESP8266_SendAT(cmd);//发送内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：命令下传：关灯
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdLedOff\\\"\\}}}\","
					"0,0",
					id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止发送过于频繁
					ESP8266_SendAT(cmd);//发送内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
				
				
				else//如果收到的数据只包含LED
				{
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：命令下传：灯命令失败
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdLedFailed\\\"\\}}}\","
					"0,0",id);
					
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止方式过于频繁
					ESP8266_SendAT(cmd);//发送内容
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
			}

			else if(strstr(line, "lock"))//如果接收的内容包含lock
			{
					if(strstr(line,"true"))//如果接收的内容还包含true
				{
					Lock_ON();//电磁锁上锁

					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，避免发送过于频繁
					Send_Lock_Status_To_OneNET(id);//发送电磁锁的状态信息
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：命令下传：电磁锁上锁
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdLockOn\\\"\\}}}\","
					"0,0",id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止发送过于频繁
					ESP8266_SendAT(cmd);//发送内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
				else if(strstr(line,"false"))//如果收到的数据还包含的是false
				{
					Lock_OFF();//电磁锁开锁
				
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，避免发送过于频繁
					Send_Lock_Status_To_OneNET(id);//发送电磁锁的状态信息
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：命令下传：电磁锁开锁
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdLockOff\\\"\\}}}\","
					"0,0",
					id);
					
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止发送过于频繁
					ESP8266_SendAT(cmd);//发送内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
				else//如果收到的数据只包含了lock
				{
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：电磁锁命令失败
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdLockFailed\\\"\\}}}\","
					"0,0",id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止发送过于频繁
					ESP8266_SendAT(cmd);//发送格式化内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量									
				}
			}
			

			else if(strstr(line, "waterPump"))//如果收到的数据包含的是waterPump
			{
					if(strstr(line,"true"))//如果收到的数据还包含了true
				{
					Waterbom_ON();//开启水泵
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止发送过于频繁
					Send_WaterPump_Status_To_OneNET(id);//发送水泵的状态信息
					id++;//id加一，防止发送过于频繁
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化内容：命令下传：水泵开
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdWaterPumpOn\\\"\\}}}\","
					"0,0",id);
					
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒
					ESP8266_SendAT(cmd);//发送格式化内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
				
				else if(strstr(line,"false"))//如果接收到的数据还包含的是false
				{
					Waterbom_OFF();//关闭水泵
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止重复
					Send_WaterPump_Status_To_OneNET(id);//发送水泵的状态信息
					id++;//id加一，防止重复
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化内容：命令下传：水泵关
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdWaterPumpOff\\\"\\}}}\","
					"0,0",id);
					
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止重复
					ESP8266_SendAT(cmd);//发送格式化内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
				
				else//如果接收到的数据只包含waterPump
				{
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化内容：命令下传：水泵命令失败
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdWaterPumpFailed\\\"\\}}}\","
					"0,0",
					id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止重复
					ESP8266_SendAT(cmd);//发送格式化内容
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
			}
			

			else if(strstr(line, "door"))//如果收到的数据包含的door
			{
					if(strstr(line,"closeIn"))//如果收到的数据还包含了closeIn
				{
					DoorControl_OutToIn_Close();//门向内关
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止重复
					Send_Door_Status_To_OneNET(id);//发送门的状态信息
					id++;//id加一，防止重复
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：门：向内开
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdClIn\\\"\\}}}\","
					"0,0",id);
					
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止重复
					ESP8266_SendAT(cmd);//发送格式化内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
				
				
				else if(strstr(line,"openIn"))//如果接收到的数据还包含的是openIn
				{
					this_status_lock = getStatus_lock();//获取电磁锁的状态信息
					
					if(this_status_lock)	//如果电磁锁锁处于上锁的状态，应先开锁再开门
					{
						Lock_OFF();//电磁锁开锁
					}
					
					DoorControl_OutToIn_Open();//门向外开
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止重复
					Send_Door_Status_To_OneNET(id);//发送格式化内容
					id++;//id加一，防止重复	
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：命令下传：门：向内开
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdOpIn\\\"\\}}}\","
					"0,0",id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止重复
					ESP8266_SendAT(cmd);//发送格式化内容
					id++;//id加一，防止重复					
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					if(this_status_lock)//如果真的上锁了，因为前面已经开锁了，所以还要向上位机上传门锁的新状态
					{
						xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
						vTaskDelay(3000);//延时3秒，防止重复
						Send_Lock_Status_To_OneNET(id);//发送格式化内容
						id++;//id加一，防止重复
						xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					}
				}
				
				else if(strstr(line,"closeOut"))//如果接收到的数据还包含的是closeOut
				{
					DoorControl_InToOut_Close();//门向外关
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止重复
					Send_Door_Status_To_OneNET(id);//发送格式化内容
					id++;//id加一，防止重复	
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：命令下传：门向外关
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdClOut\\\"\\}}}\","
					"0,0",id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止重复
					ESP8266_SendAT(cmd);//发送格式化内容	
					id++;//id加一，防止重复					
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
				
				
				else if(strstr(line,"openOut"))//如果收到的数据还包含的是openOut
				{
					this_status_lock = getStatus_lock();//获取电磁锁的状态信息
					
					if(this_status_lock)//如果电磁锁处于上锁的状态，应先开锁再开门
					{
						Lock_OFF();//电磁锁开锁
					}
					
					DoorControl_InToOut_Open();//门由内向外开
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止重复
					Send_Door_Status_To_OneNET(id);//发送格式化内容
					id++;//id加一，防止重复
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：命令下传：门向外开
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdOpOut\\\"\\}}}\","
					"0,0",id);
					
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止重复
					ESP8266_SendAT(cmd);//发送格式化内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					if(this_status_lock)//如果真的上锁了，因为前面已经开锁了，所以还要向上位机上传门锁的新状态
					{
						xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
						vTaskDelay(3000);//延时3秒，防止重置
						Send_Lock_Status_To_OneNET(id);//发送格式化内容
						id++;//id加一，防止重复
						xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					}
				}
				
				else
				{
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：命令下传：门命令失败
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdDoorFailed\\\"\\}}}\","
					"0,0",id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止重置
					ESP8266_SendAT(cmd);//发送格式化内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
			}
			
			else if(strstr(line, "method"))//如果接收到的数据包含的是method
			{
					if(strstr(line,"manual"))//如果接收到的数据还包含了manual
					{
//					printf("开启手动模式\r\n");
						//清空第十二位事件标志位，启动手动控制模式
						xEventGroupClearBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<12);
						
						memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
						
						//格式化发送内容：设备管理方式：手动
						sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
						"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
						"\\\"method\\\":{\\\"value\\\":\\\"manual\\\"\\}}}\","
						"0,0",id);
						
						xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
						vTaskDelay(3000);//延时3秒，防止发送过于频繁
						ESP8266_SendAT(cmd);	//发送格式化内容
						id++;//id加一，防止重复
						xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
						
						memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
						
						//格式化发送内容
						sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
						"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
						"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdManual\\\"\\}}}\","
						"0,0",id);
						
						
						xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
						vTaskDelay(3000);//延时3秒，防止发送过于频繁
						ESP8266_SendAT(cmd);//发送格式化内容	
						id++;//id加一，防止重复
						xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					}
					
					else if(strstr(line,"auto"))//如果接收到的数据还包含的是auto
					{					
//					printf("开启自动模式\r\n");
						xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<12);//第12位事件标志位置一，开启自动控制模式
						
						memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
						
						//格式化发送内容：设备管理方式：自动
						sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
						"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
						"\\\"method\\\":{\\\"value\\\":\\\"auto\\\"\\}}}\","
						"0,0",id);
						
						xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
						vTaskDelay(3000);//延时3秒，防止发送过于频繁
						ESP8266_SendAT(cmd);	//发送格式化内容
						id++;//id加一，防止重复
						xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
						
						memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
						
						//格式化发送内容：命令下传：自动命令
						sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
						"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
						"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdAuto\\\"\\}}}\","
						"0,0",id);
						
						xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
						vTaskDelay(3000);//延时3秒，防止发送过于频繁
						ESP8266_SendAT(cmd);//发送格式化内容
						id++;//id加一，防止重复
						xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					}
					
				else//如果接收到的数据只包含了method
				{
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：命令下传：设备管理方式命令失败
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdMethodFailed\\\"\\}}}\","
					"0,0",id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止发送过于频繁
					ESP8266_SendAT(cmd);//发送格式化内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
			}
			
			else if(strstr(line, "reset"))//如果接收到的数据包含的是reset
			{
					if(strstr(line,"true"))//如果接收到的数据还包含的是true
				{																	
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：重置：重置中
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"reset\\\":{\\\"value\\\":true\\}}}\","
					"0,0",id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止发送过于频繁
					ESP8266_SendAT(cmd);//发送格式化内容
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
					
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdReset\\\"\\}}}\","
					"0,0",id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止发送过于频繁
					ESP8266_SendAT(cmd);//发送格式化内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
										
					NVIC_SystemReset();// 触发系统复位
				}
				
				else//如果接收到的数据只包含了reset
				{
					memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：命令下传：重置命令失败
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdResetFailed\\\"\\}}}\","
					"0,0",id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止发送过于频繁
					ESP8266_SendAT(cmd);//发送格式化内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				}
			}
			
			else
			{
				memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
					
					//格式化发送内容：命令下传：未知命令
					sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
					"\\\"cmdPassDown\\\":{\\\"value\\\":\\\"cmdUnknown\\\"\\}}}\","
					"0,0",id);
					
					xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
					vTaskDelay(3000);//延时3秒，防止发送过于频繁
					ESP8266_SendAT(cmd);//发送格式化内容	
					id++;//id加一，防止重复				
					xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
			}
		}
			
        vTaskDelay(10);//延迟10ms给其他任务执行时间
    }
}


//软件定时器softTimer的回调函数
void callBackFunc_softTimer(TimerHandle_t xTimer)
{
	//是否重新执行开关门和定时器的标志位
	firstFlag_softTimer = 1;
	
	//关闭舵机门开启和门内外红外光电传感器功能
	xEventGroupClearBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<1);
	
	//向外关门
	DoorControl_InToOut_Close();
	
	xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
	vTaskDelay(3000);//延时3秒，防止发送过于频繁
	Send_Door_Status_To_OneNET(id);//上传门的状态信息
	xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
	id++;//id加一，防止重复
	
	//将事件标志位第一位置一，表示重新开启门外人体红外热释电传感器检测
	xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<0);
}



//第一位 1<<0 任务：人体红外热释电传感器
void task_bodyHw(void* pvParameters)
{
	while(1)
	{
		//等待卫生间自动开关门事件标志组第一位置一
		xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<0, pdFALSE, pdFALSE, portMAX_DELAY);
		
		//判断是否为自动控制
		xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<12, pdFALSE, pdFALSE, portMAX_DELAY);
		bodyHw_value_out = BODY_HW_GetData_out();	//获取当前人体红外热释电传感器检测值

		if(bodyHw_value_out == 1)	//如果当前值为1，表示检测到有人靠近
		{	 
			xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<1);	//开启舵机门向内开和门内外红外光电传感器检测的功能
					
			xEventGroupClearBits(handle_eventGroup_servo_openOrCloseCtrl,	1<<0);	//关闭门外人体红外传感器检测的功能
		}
		else	//否则表示没有检测到有人靠近
		{
					
		}
		
		vTaskDelay(10);//延时10ms，给其他任务执行时间
	}
}



//第二位 1<<1 任务：舵机门向内开
void task_servo_openToIn(void *pvPrameters)
{
	
	while(1)
	{
		//等待事件标志组：舵机门向内开控制
		//参数一：事件标志组句柄
		//参数二：等待事件标志位（第一位）
		//参数三：返回前是否清除标志位（是）
		//参数四：是否等待所有标志位置1（否）
		//参数五：等待时间（最大）
		xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<1, pdFALSE, pdFALSE, portMAX_DELAY);
		
		//判断是否为自动控制
		xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<12, pdFALSE, pdFALSE, portMAX_DELAY);
				
		if(firstFlag_softTimer == 1)//如果软件定时器是第一次触发//else部分见软件定时器回调函数
		{
			//舵机门向内开具体实现函数
			DoorControl_OutToIn_Open();
			
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
			vTaskDelay(3000);//延时3秒，防止发送过于频繁
			Send_Door_Status_To_OneNET(id);//上传门的状态信息
			xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
			id++;//id加一，防止重复			
	
			xTimerStart(handle_softTimer, portMAX_DELAY);//软件定时器开始计时
			xTimerReset(handle_softTimer,	portMAX_DELAY);//重置软件定时器
			firstFlag_softTimer = 0;//将软件定时器初次触发标志位置零
		}
		
		//-------------likely danger code--------------
		bodyHw_value_in= BODY_HW_GetData_in();	//获取当前人体红外热释电传感器检测值（门内）

		if(bodyHw_value_in != bodyHw_value_pre_in)	//如果当前值与之前值不同
		{
			if(bodyHw_value_in == 1)	//如果当前值为1，表示检测到有人进入卫生间内
			{
				state_user_in = HAS_COME_IN;//将使用者标志位赋值为已经完全进入
			}
			else	//否则表示没有检测到有人进入卫生间内
			{
				state_user_in = HAS_NO_COME_IN;//将使用者标志位赋值为还没进入
			}
		}

		bodyHw_value_pre_in = bodyHw_value_in;	//将当前值赋值给之前值
		//-------------likely danger code--------------
		
		if(HW_GetData_OUT() == 0 && HW_GetData_IN() == 0 && state_user_in == HAS_COME_IN)//使用者已完全进入卫生间
		{
			xTimerStop(handle_softTimer, portMAX_DELAY);	//定时器停止计时
			firstFlag_softTimer = 1;//将软件定时器初次触发标志位置一
			
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
			vTaskDelay(3000);//延时3秒，防止发送过于频繁
			Send_User_Status_To_OneNET(id);//上传使用者的状态信息
			xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
			id++;//id加一，防止重复
			
			DoorControl_InToOut_Close();	//舵机门由内向外关
			
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
			vTaskDelay(3000);//延时3秒，防止发送过于频繁
			Send_Door_Status_To_OneNET(id);//上传门的状态信息
			xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
			id++;//id加一，防止重复
							
			Lock_ON();	//电磁锁上锁
			
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
			vTaskDelay(3000);//延时3秒，防止发送过于频繁
			Send_Lock_Status_To_OneNET(id);//上传电磁锁的状态信息
			xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
			id++;//id加一，防止重复
			
			xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<7);	//启动卫生间使用时间计时
			xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl,	1<<13);	//启动求救按键任务
			xEventGroupClearBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<1);	//关闭第二位任务
		}
		else if(HW_GetData_OUT() == 1 || HW_GetData_IN() == 1)//如果任意一个红外对管传感器被遮挡
		{
			xTimerReset(handle_softTimer, portMAX_DELAY);	//都会使定时器重新计时
			//设计目的：如果使用者遮挡了任意一个红外对管传感器就表明其位置还处于门的开关范围内，因此需要要让定时器重新计时
		}
		
		vTaskDelay(10);//延时10ms，给其他任务执行的时间
	}
}



//第三位 1<<2 任务：舵机门向外开
void task_servo_openToOut(void *pvPrameters)
{
	while(1)
	{
		//等待事件标志组：舵机门向内开控制
		//参数一：事件标志组句柄
		//参数二：等待事件标志位（第一位）
		//参数三：返回前是否清除标志位（否）
		//参数四：是否等待所有标志位置1（否）
		//参数五：等待时间（最大）
		xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<2, pdFALSE, pdFALSE, portMAX_DELAY);
		
		//判断是否为自动控制
		xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<12, pdFALSE, pdFALSE, portMAX_DELAY);

		safety_timer_stop();//卫生间使用时间定时器停止计时
		state_first_safety = 1;//卫生间使用时间定时器首次触发标志位
		xEventGroupClearBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<7);//清空第8位事件标志位，停止卫生间使用时间定时器任务
			
		Lock_OFF();	//开锁
		
		xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
		vTaskDelay(3000);//延时3秒，防止发送过于频繁
		Send_Lock_Status_To_OneNET(id);//上传电磁锁的状态信息
		xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
		id++;//id加一，防止重复
			
		//舵机门向外开具体实现函数
		DoorControl_InToOut_Open();
		
		xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
		vTaskDelay(3000);//延时3秒，防止发送过于频繁
		Send_Door_Status_To_OneNET(id);//上传门的状态信息
		xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
		id++;//id加一，防止重复
		
		state_user_in = HAS_NO_COME_IN;//使用者标志位赋值为还没进入
		
		//开始舵机门向内开任务
		xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<3);
		
		//停止本任务
		xEventGroupClearBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<2);
		
		//延时10ms，给其他任务执行时间
		vTaskDelay(10);
	}
}



//第四位 1<<3 任务：舵机门向内关
void task_servo_closeToIn(void* pvParameters)
{
	while(1)
	{
		//等待本任务的执行事件标志位置一
		xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<3, pdFALSE, pdFALSE, portMAX_DELAY);
		
		//判断是否为自动控制
		xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<12, pdFALSE, pdFALSE, portMAX_DELAY);
		
		//如果使用者没有遮挡门内外红外光电传感器，且卫生间内的人体红外热释电传感器也没有感应到卫生间内有人。
		if((HW_GetData_IN() != 1) && (HW_GetData_OUT() != 1) && (BODY_HW_GetData_in() != 1))
		{
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
			vTaskDelay(3000);//延时3秒，
			Send_User_Status_To_OneNET(id);//上传使用者的状态信息
			xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
	
			DoorControl_OutToIn_Close();//门由外向里关
			
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
			vTaskDelay(3000);//延时3秒，防止发送过于频繁
			Send_Door_Status_To_OneNET(id);//上传门的状态信息
			xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
			id++;//id加一，防止重复
			
			xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl,	1<<4);//开启水泵冲水任务
			
			xEventGroupClearBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<3);//关闭本任务
			
		}
		
		vTaskDelay(10);//延时10ms，给其他任务执行时间
	}
}



//第五位 1<<4 任务：水泵冲水
void task_waterPump(void* pvParameters)
{
	while(1)
	{
		//等待本任务的执行事件标志位置一
		xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<4, pdFALSE, pdFALSE, portMAX_DELAY);

		
		//判断是否为自动控制
		xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<12, pdFALSE, pdFALSE, portMAX_DELAY);
		
		Waterbom_ON();//开启水泵
		
		xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
		vTaskDelay(3000);//延迟3秒，防止发送过于频繁
		Send_WaterPump_Status_To_OneNET(id);//上传水泵的状态信息
		xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
		id++;//id加一，防止重复		
		
		vTaskDelay(5000);//冲水时间3+5=8秒
		
		
		Waterbom_OFF();//关闭水泵
		
		xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
		vTaskDelay(3000);//延时3秒，防止发送过于频繁
		Send_WaterPump_Status_To_OneNET(id);//上传水泵的状态信息
		xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
		id++;//id加一，防止重复							
		
		xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<0);//开启门外人体红外热释电传感器HC-SR501检测任务
		xEventGroupClearBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<4);//停止本任务
		
		vTaskDelay(10);//延时10ms，给其他任务执行时间
	}
}

//第十二位任务：检测异味气体浓度
void task_wind_src(void* pvParameters)
{
	//等待本任务的执行事件标志位置一
	xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<11, pdFALSE, pdFALSE, portMAX_DELAY);
	
	//用于接收传感器检测到的数据
	uint16_t temp_mq135_data = 0;
	
	//异味气体浓度过高标志位
	uint8_t flag_air_big = 0;
	
	//异味气体浓度正常标志位
	uint8_t flag_air_small = 0;
	
	//定义一个发送缓冲区
	char cmd[250];
	
	while(1)
	{
		xSemaphoreTake(handle_semMutex_mq135, portMAX_DELAY);//获取空气质量传感器MQ-135的互斥信号量
		temp_mq135_data = MQ135_GetData();//接收MQ-135检测到的数据
		xSemaphoreGive(handle_semMutex_mq135);//释放MQ-135的互斥信号量
		
		if(temp_mq135_data > 2000)//如果异味气体浓度过高
		{
			if(flag_air_big == 0)	//如果之前没有上传过异味气体浓度大于2000的消息
			{	//那就上传
				memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
				
				//格式化发送内容：异味气体浓度：过高
				sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
								"\\\"air\\\":{\\\"value\\\":\\\"high\\\"\\}}}\","
								"0,0",id);
				
				xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
				vTaskDelay(3000);//延迟3秒，防止发送过于频繁
				ESP8266_SendAT(cmd);//发送格式化内容	
				id++;//id加一，防止重复
				xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				
				flag_air_big = 1;	//已经上传过异味气体浓度大于2000的消息了
				flag_air_small = 0;//异味气体浓度正常标志位置零表示还没上传过
			}
		}
		else if(temp_mq135_data < 2000)//如果异味气体浓度正常
		{
			if(flag_air_small == 0)	//如果之前没有上传过异味气体浓度小于2000的消息
			{	//那就上传
				memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
				
				//格式化发送内容：异味气体浓度：过高
				sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
								"\\\"air\\\":{\\\"value\\\":\\\"normal\\\"\\}}}\","
								"0,0",id);
				
				xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
				vTaskDelay(3000);//延时3秒，防止发送过于频繁
				ESP8266_SendAT(cmd);//发送格式化内容	
				id++;//id加一，防止发送过于频繁
				xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				
				flag_air_big = 0;	//异味气体浓度过高标志位置零表示还没上传过
				flag_air_small = 1;	//已经上传过异味气体浓度小于2000的消息了
			}
		}
		
		vTaskDelay(100);//延时100ms，给其他任务执行时间
	}
}

//第十二位任务：风扇通风
void task_wind(void* pvParameters)
{
	//等待本任务的执行事件标志位置一
	xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<11, pdFALSE, pdFALSE, portMAX_DELAY);
	
	uint8_t on_wind = 0;//风扇初次启动标志位
	uint8_t off_wind = 0;//风扇初次关闭标志位
	
	uint16_t temp_value_mq135 = 0;//接收MQ-135检测的数据
	
	char cmd[250];//定义一个发送缓冲区
	
	while(1)
	{
		//判断是否为自动控制
		xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<12, pdFALSE, pdFALSE, portMAX_DELAY);
		
		xSemaphoreTake(handle_semMutex_mq135, portMAX_DELAY);//获取MQ-135的互斥信号量
		temp_value_mq135 = MQ135_GetData();//接收MQ-135检测的数据
		xSemaphoreGive(handle_semMutex_mq135);//释放MQ-135的互斥信号量
		
		if(temp_value_mq135 > 2000)//如果异味气体浓度过高
		{
			if(on_wind == 0)//如果风扇还没开启
			{
				Wind_ON();//开启风扇
				memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
				
				//格式化发送内容：风扇的状态信息
				sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
								"\\\"wind\\\":{\\\"value\\\":%s\\}}}\","
								"0,0",id,getStatus_wind() ? "true" : "false");
				
				xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
				vTaskDelay(3000);//延时3秒，防止发送过于频繁
				ESP8266_SendAT(cmd);//上传格式化内容	
				id++;//id加一，防止重复
				xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
		
				on_wind = 1;//风扇开启标志位置一，表示风扇已经开启
				off_wind = 0;//风扇关闭标志位置零，表示风扇还未关闭
			}
		}
		else//如果异味气体浓度正常
		{
			if(off_wind == 0)//如果风扇还未关闭
			{
				Wind_OFF();//风扇关闭
				
				memset(cmd, 0, sizeof(cmd));//清空发送缓冲区
				
				//格式化发送内容：风扇：状态信息
				sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
        "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
				"\\\"wind\\\":{\\\"value\\\":%s\\}}}\","
				"0,0",id,getStatus_wind() ? "true" : "false");
				
				
				xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
				vTaskDelay(3000);//延时3秒，防止发送过于频繁
				ESP8266_SendAT(cmd);//发送格式化内容
				id++;	//id加一，防止重复
				xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				
				on_wind = 0;//风扇开启标志位置零表示风扇还未开启
				off_wind = 1;//风扇关闭标志位置一表示风扇已经关闭
			}
		}
		
		vTaskDelay(100);//延时100ms，给其他任务执行的时间
	}
}



//第八位 1<<7 任务：卫生间使用时间计时
void task_safety(void* pvParameters)
{
	//定义一个发送缓冲区
	char cmd4[sizeof("AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"126\\\"\\,\\\"params\\\":{"
								"\\\"timeOut\\\":{\\\"value\\\":123456\\}}}\","
								"0,0")];
		while(1)
		{
			//等待本任务的执行事件标志位置一
			xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<7, pdFALSE, pdFALSE, portMAX_DELAY);
		
			//判断是否为自动控制
			xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<12, pdFALSE, pdFALSE, portMAX_DELAY);
			
			if(state_first_safety == 1)//避免while循环重复执行此部分代码
			{
				safety_timer_reset();//卫生间使用时间定时器重置
				safety_timer_start();//卫生间使用时间定时器开启
				state_first_safety = 0;//卫生间使用时间定时器标志位置0，表示定时器已经开启
				printf("yes\r\n");//调试信息，确认卫生间使用时间定时器是否开启
			}
			
			if(safety_timer_get_accident_flag() == 1)//如果卫生间使用时间超时
			{
				//格式化发送内容：卫生间使用时间：超时
				sprintf(cmd4, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"timeOut\\\":{\\\"value\\\":%s\\}}}\","
								"0,0",
								id,safety_timer_get_accident_flag() ? "true":"false");
				
				xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);//获取串口1的互斥信号量
				vTaskDelay(3000);//延时3秒，防止发送过于频繁
				ESP8266_SendAT(cmd4);//发送格式化内容
				xSemaphoreGive(handle_semMutex_usart);//释放串口1的互斥信号量
				id++;//id加一，防止重复
				
				state_first_safety = 1;//将卫生间使用时间定时器标志位置一表示定时器已关闭
				
				//停止本任务
				xEventGroupClearBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<7);
				//等待管理员处理
			}
			
			if(HW_GetData_IN() == 1)//使用者要出去
			{
				xEventGroupSetBits(handle_eventGroup_servo_openOrCloseCtrl,	1<<2);	//启动第三位任务让门向外打开
				xEventGroupClearBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<13);	//关闭求救按键任务
				xEventGroupClearBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<7);	//关闭自身
			}
						
			vTaskDelay(10);//延时10ms，给其他任务执行的时间
		}
}



//第七位任务：检测环境亮度
void task_light_src(void *pvParameters)
{
	//等待本任务执行的事件标志位置一
	xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<6, pdFALSE, pdFALSE, portMAX_DELAY);
	
	//定义一个发送缓冲区
	char cmd[sizeof( "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
								"\\\"lighting\\\":{\\\"value\\\":\\\"1234567\\\"\\}}}\","
								"0,0") + 5];
	
	//环境光照强度亮标志位
	uint8_t flag_lighting_bright = 0;
	
	//环境光照强度暗标志位
	uint8_t flag_lighting_dark = 0;
	
	//接收环境光照强度亮暗判断函数返回值
	uint8_t temp_data_lighting = 0;
	
	while(1)
	{
		//获取光敏电阻传感器的互斥信号量
		xSemaphoreTake(handle_semMutex_lighting, portMAX_DELAY);
		//获取亮暗判断函数返回值
		temp_data_lighting = Light_IsDark();
		//释放光敏电阻传感器的互斥信号量
		xSemaphoreGive(handle_semMutex_lighting);
		
		if(temp_data_lighting == 1)//如果环境光照强度暗
		{
			if(flag_lighting_dark == 0)//如果环境光照强度暗标志位为0，表示还没上传暗的状态信息
			{	
				//格式化发送内容：环境光照强度：暗
				sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
								"\\\"lighting\\\":{\\\"value\\\":\\\"dark\\\"\\}}}\","
								"0,0",id);
				
				//获取串口1的互斥信号量
				xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);
				//延时3秒，防止发送过于频繁
				vTaskDelay(3000);
				//发送格式化内容
				ESP8266_SendAT(cmd);
				//释放串口1的互斥信号量
				xSemaphoreGive(handle_semMutex_usart);
				
				//环境光照强度暗标志位置一，表示已上传暗的状态信息，防止上传重复的状态信息
				flag_lighting_dark = 1;
				//环境光照强度亮标志位置零，表示未上传亮的状态信息，使其能够在状态信息发送改变，上传新的状态信息
				flag_lighting_bright= 0;
			}
		}
		//如果亮暗判断返回值为0，表示亮
		else if(temp_data_lighting == 0)
		{
			//如果环境光照强度亮标志位为0，表示未上传亮的状态信息
			if(flag_lighting_bright == 0)
			{
				//格式化发送内容
				sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
								"\\\"lighting\\\":{\\\"value\\\":\\\"bright\\\"\\}}}\","
								"0,0",id);
				
				//获取串口1的互斥信号量
				xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);
				//延时3秒，防止发送过于频繁
				vTaskDelay(3000);
				//发送格式化内容
				ESP8266_SendAT(cmd);
				//id加一，防止重复
				id++;
				//释放串口1的互斥信号量
				xSemaphoreGive(handle_semMutex_usart);

				//环境光照强度亮标志位为1，表示已上传亮的状态标志位，防止上传重复的状态信息
				flag_lighting_bright = 1;
				//环境光照强度暗标志位置零，表示未上传暗的状态信息，使其能够在状态信息发送改变，上传新的状态信息
				flag_lighting_dark = 0;
			}
		}
		//延时10ms，给其他任务执行的时间
		vTaskDelay(10);
	}
}


//任务：自动开关灯
void task_light(void *pvParameters)
{
	//等待本任务执行的事件标志位置一
	xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<6, pdFALSE, pdFALSE, portMAX_DELAY);
	
	//灯开标志位
	uint8_t on_led = 0;
	//灯关状态信息上传标志位
	uint8_t off_led = 0;
	//定义一个发送缓冲区
	char cmd[sizeof( "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
								"\\\"LED\\\":{\\\"value\\\":123456\\}}}\","
								"0,0") + 5];
	
	while(1)
	{		
		//判断是否为自动控制
		xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<12, pdFALSE, pdFALSE, portMAX_DELAY);
		
		//获取光敏电阻传感器的互斥信号量
		xSemaphoreTake(handle_semMutex_lighting, portMAX_DELAY);
		//获取判断亮暗函数返回值
		uint8_t isDark = Light_IsDark();
		//释放光敏电阻传感器的互斥信号量
		xSemaphoreGive(handle_semMutex_lighting);

		//isDark为1表示亮暗判断函数返回值为暗
		if(isDark == 1)
		{
			//如果灯开标志位为0，表示灯未开，未上传灯开的状态信息
			if(on_led == 0)
			{
				//开灯
				LED_ON();
				
				//格式化发送内容：灯：开
				sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
								"\\\"LED\\\":{\\\"value\\\":true\\}}}\","
								"0,0",id);
				
				//获取串口1的互斥信号量
				xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);
				//延时3秒，防止发送过于频繁
				vTaskDelay(3000);
				//发送格式化内容
				ESP8266_SendAT(cmd);
				//释放串口1的互斥信号量
				xSemaphoreGive(handle_semMutex_usart);
				
				//灯开状态标志位为1，表示灯已开，灯开的状态信息已上传
				on_led = 1;
				//灯关状态标志位为0，表示灯未关，灯关的状态信息未上传
				off_led = 0;
			}
		}
		
		else//如果亮暗判断函数返回值为0，表示环境光照强度亮
		{
			if(off_led == 0)//如果灯关状态标志位为0，表示灯未关，灯关的状态信息未上传
			{
				//关灯
				LED_OFF();
				
				//格式化发送内容：灯：开
				sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{"
								"\\\"LED\\\":{\\\"value\\\":false\\}}}\","
								"0,0",id);
				
				//获取串口1的状态标志位
				xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);
				//延时3秒，防止发送过于频繁
				vTaskDelay(3000);
				//发送格式化内容
				ESP8266_SendAT(cmd);
				//id加一，防止重复
				id++;
				//释放串口1的状态标志位
				xSemaphoreGive(handle_semMutex_usart);
				
				//灯开标志位为0，表示灯未开，灯开的状态信息未上传
				on_led = 0;
				//灯关标志位为1，表示灯已关，灯关的状态信息已上传
				off_led = 1;
			}
		}
		
		//延时10ms，给其他任务执行的时间
		vTaskDelay(10);
	}
}



// 第十位 任务：检测卫生间内水位
void task_waterLevel(void* pvParameters)
{
	//等待本任务执行的事件标志位置一
	xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<9, pdFALSE, pdFALSE, portMAX_DELAY);
	
	//定义一个发送缓冲区
	char cmd[250];
	
	
	uint8_t firstFlag_seventhBits_set = 0; //水位高度正常标志位，为0表示水位高度正常的状态信息未上传，反之已上传
	uint8_t firstFlag_seventhBits_reset = 0; //水位高度过高标志位，为0表示水位高度过高的状态信息未上传，反之已上传
	while(1)
	{
		//读取水位传感器检测数据
		uint16_t waterLevel_this = Water_ADC_Read();
		
		//如果水位高度正常且水位高度正常标志位为0
		if((waterLevel_this < 1000) && (firstFlag_seventhBits_set == 0))
		{
			firstFlag_seventhBits_set = 1; //防止反复执行该部分代码
			firstFlag_seventhBits_reset = 0; //如果水位高度状态信息改变，可以及时上传新的状态信息
			
			//格式化发送内容：水位高度：正常
			sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"waterLevel\\\":{\\\"value\\\":\\\"normal\\\"\\}}}\","
								"0,0", id);
				
			//获取串口1的互斥信号量
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);
			//调试信息，观察程序是否有执行该部分代码
			printf("水位正常\r\n");
			//延时3秒，防止发送过于频繁
			vTaskDelay(3000);
			//发送格式化内容
			ESP8266_SendAT(cmd);
			//id加一，防止重复
			id++;
			//释放串口1的互斥信号量
			xSemaphoreGive(handle_semMutex_usart);
		}
		
		//水位高度过高并且水位高度过高标志位为0
		else if((waterLevel_this >= 1000) && (firstFlag_seventhBits_reset == 0))
		{		
			//格式化发送内容：水位高度：过高
			sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
                "\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"waterLevel\\\":{\\\"value\\\":\\\"high\\\"\\}}}\","
								"0,0", id);
			
			//获取串口1的互斥信号量	
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);
			//提示信息，验证程序释放有执行此部分代码
			printf("水位过高\r\n");
			//延时3秒，防止发送过于频繁
			vTaskDelay(3000);
			//发送格式化内容
			ESP8266_SendAT(cmd);
			//id加一，防止重复
			id++;
			//释放串口1的互斥信号量
			xSemaphoreGive(handle_semMutex_usart);
			
			firstFlag_seventhBits_set = 0; //防止反复执行该部分代码
			firstFlag_seventhBits_reset = 1; //防止反复执行该部分代码
		}
					
		//延时100ms，给其他任务执行时间
		vTaskDelay(100);
	}
}



//任务：OLED显示
void task_oled(void* pvParameters)
{
	//等待本任务执行的事件标志位置一
	xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<10, pdFALSE, pdFALSE, portMAX_DELAY);
	while(1)
	{
//		state_waterLevel state_waterLevel_this = waterLevel_isHigh();
		//读取水位传感器的检测数据
		uint16_t waterLevel_this = Water_ADC_Read();
		
		//如果水位高度过高
		if(waterLevel_this > 1000)
		{
			//如果OLED状态位不为积水状态
			if(state_oled_this != DONE_STANDING_WATER)
			{
				OLED_Clear();//清屏
			}
			
			OLED_ShowChinese(1, 4, 0);	//积
			OLED_ShowChinese(1, 5, 1);	//水
//		OLED_ShowString(1, 2, "Standing Water");
			state_oled_this = DONE_STANDING_WATER;//将OLED状态位赋值为积水状态，防止反复清屏，延长OLED的使用寿命
		}
		
		//如果水位高度正常
		else
		{
			if(state_user_in == HAS_COME_IN)//如果使用者状态标志位为已经进入
			{
				if(state_oled_this != DONE_USING)//如果OLED状态标志位不为正在使用中
				{
					OLED_Clear();//清屏
				}
	
			OLED_ShowChinese(1, 4, 2);	//有
			OLED_ShowChinese(1, 5, 3);	//人				
//		OLED_ShowString(1, 6, "USING");
			state_oled_this = DONE_USING;//将OLED状态标志位赋值为正在使用中，防止反复清屏，延长OLED的使用寿命
			}
			
			//如果使用者状态不是已经进入
			else
			{
				if(state_oled_this != DONE_FREE)//如果OLED状态标志位不为空闲状态
				{
					OLED_Clear();//清屏
				}

			OLED_ShowChinese(1, 4, 4);	//空
			OLED_ShowChinese(1, 5, 5);	//闲	
//				OLED_ShowString(1, 7, "FREE");
				state_oled_this = DONE_FREE;//将OLED状态标志位赋值为空闲状态，防止反复清屏，延长OLED的使用寿命
			}
		}
		
		vTaskDelay(100);//延时100ms，给其他任务执行时间
	}
}

//检测纸巾剩余量任务
void task_weight(void* pvParameters)
{
	//等待本任务执行的事件标志位置一
	xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<8, pdFALSE, pdFALSE, portMAX_DELAY);
	
	//定义一个发送缓冲区
	char cmd[250];
	
	//纸巾剩余量状态标志位
	//为0表示初始值，无任何意义
	//为1表示纸巾重量>75g
	//为2表示75g>=纸巾重量>50g
	//为3表示50g>=纸巾重量>25g
	//为4表示25g>=纸巾重量
	uint8_t flag_weight_set = 0;
	
	while(1)
	{
		//读取HX711称重模块数据
		value = HX711_GetData();
//		printf("%d\r\n",value);
		
		//通过公式计算出重量
		weight=(float)(value-reset)*Weights/(float)(Weights_100-reset);
		
		if(weight < 0)//如果计算出来的重量小于0g
		{
			weight = 0;//就将其赋值为0		
		}
//		printf("%.1f\r\n",weight);
//		memset(buff, 0, sizeof(buff));
//		sprintf((char*)buff, "%.1f", weight);

		//如果重量大于75g且纸巾重量状态标志位不为1
		if(weight > 75 && (flag_weight_set != 1))
		{
				//将纸巾状态标志位赋值为1，防止反复执行此部分代码
				flag_weight_set = 1;
				
				//格式化发送内容：纸巾剩余量：很多
				sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
								"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"paper\\\":{\\\"value\\\":\\\"many\\\"\\}}}\","
								"0,0", id);
			
				//获取串口1的互斥信号量
				xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);
				//调试信息，用于判断程序是否有执行此部分代码
				printf("很多\r\n");
				//延时3秒，防止发送过于频繁
				vTaskDelay(3000);
				//发送格式化内容
				ESP8266_SendAT(cmd);
				//id加一，防止重复
				id++;
				//释放串口1的互斥信号量
				xSemaphoreGive(handle_semMutex_usart);
		}
		
		//如果重量大于50但小于等于75并且纸巾剩余量标志位不为2
		else if(weight <= 75 && weight > 50 && (flag_weight_set != 2))
		{
				//将纸巾剩余量标志位赋值为2，防止重复执行此部分代码
				flag_weight_set = 2;
				
				//格式化发送内容：纸巾剩余量：足够
				sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
							"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"paper\\\":{\\\"value\\\":\\\"enough\\\"\\}}}\","
							"0,0", id);
			
				//获取串口1的互斥信号量
				xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);
				//调试信息，用于判断程序是否有执行此部分代码
				printf("足够\r\n");
				//延时3秒，防止发送过于频繁
				vTaskDelay(3000);
				//发送格式化内容
				ESP8266_SendAT(cmd);
				//id加一，防止重复
				id++;
				//释放串口1的互斥信号量
				xSemaphoreGive(handle_semMutex_usart);
		}
		
		//如果重量大于25g但小于等于50g并且纸巾剩余量标志位不为3
		else if(weight <= 50 && weight > 25 && (flag_weight_set != 3))
		{
			//将纸巾剩余量标志位赋值为3，防止重复执行此部分代码
			flag_weight_set = 3;
			
			//格式化发送内容：纸巾剩余量：较少
			sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
				"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"paper\\\":{\\\"value\\\":\\\"less\\\"\\}}}\","
				"0,0", id);
			
			//获取串口1的互斥信号量
			xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);
			//调试信息，用于判断程序是否有执行此部分代码
			printf("较少\r\n");
			//延时3秒，防止发送过于频繁
			vTaskDelay(3000);
			//发送格式化内容
			ESP8266_SendAT(cmd);
			//id加一，防止重复
			id++;
			//释放串口1的互斥信号量
			xSemaphoreGive(handle_semMutex_usart);
		}
		
		//如果重量小于等于25g且纸巾剩余量标志位不为4
		else if(weight <= 25 && (flag_weight_set != 4))
		{
				//将纸巾剩余量标志位赋值为4
				flag_weight_set = 4;
			
				//格式化发送内容
				sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
					"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"paper\\\":{\\\"value\\\":\\\"needy\\\"\\}}}\","
					"0,0", id);
			
				//获取串口1的互斥信号量
				xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);
				//调试信息，用于判断程序释放有执行此部分代码
				printf("极少\r\n");
				//延时3秒，防止发送过于频繁
				vTaskDelay(3000);
				//发送格式化内容
				ESP8266_SendAT(cmd);
				//id加一，防止重复
				id++;
				//释放串口1的互斥信号量
				xSemaphoreGive(handle_semMutex_usart);
		}
		//延时1秒，给其他任务执行的时间
		vTaskDelay(1000);
	}
}



//求救按键任务
void vKeyTask(void *pvParameters)
{
		//用于获取连接按键引脚的电平
    uint8_t current_level;
	
		//定义一个发送缓冲区
		char cmd[250];
	
    while (1)
    {
				//等待本任务执行的事件标志位置一
				xEventGroupWaitBits(handle_eventGroup_servo_openOrCloseCtrl, 1<<13, pdFALSE, pdFALSE, portMAX_DELAY);
        // 等待来自外部中断的按键电平变化事件
        if (xQueueReceive(xKeyEventQueue, &current_level, portMAX_DELAY) == pdTRUE)
        {
            // 消抖
            vTaskDelay(20);
						
					//二次读取按键引脚电平，与current_level进行比较，用于确认是否为抖动
            uint8_t stable_level = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);

            //如果电平不相等，就是抖动
            if (stable_level != current_level)
            {
                continue;//直接进入下一次循环
            }

            //按键状态标志位
            static uint8_t key_state = 0; // 0表示未按下，1表示已按下
						//static：第一次进入循环key_state会定义为0，此后的循环不再重复定义

						//如果按键标志位为0，但是读取的电平为高电平
            if (key_state == 0 && stable_level == 1)
            {
                key_state = 1; //更新按键标志位的值为1，表示按键按下
            }
						
						//如果按键标志位为1，但是读取读取到的电平为低电平
            else if (key_state == 1 && stable_level == 0)
            {
                key_state = 0; //更新按键标志位的值为0，表示按键松开

								//调试信息，用于确认程序是否有执行到此处
                printf("Key clicked! (Press and release)\r\n");
								
								//格式化发送内容：求救：是
								sprintf(cmd, "AT+MQTTPUB=0,\"$sys/r7vg0Nn75e/upper_computer/thing/property/post\","
								"\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"help\\\":{\\\"value\\\":true\\}}}\","
								"0,0", id);
							
								//获取串口1的互斥信号量
								xSemaphoreTake(handle_semMutex_usart, portMAX_DELAY);
								//延时3秒，防止发送过于频繁
								vTaskDelay(3000);
								//发送格式化内容
								ESP8266_SendAT(cmd);
								//释放串口1的互斥信号量
								xSemaphoreGive(handle_semMutex_usart);
								//id加一，防止重复
								id++;	
								//停止执行本任务
								xEventGroupClearBits(handle_eventGroup_servo_openOrCloseCtrl, 0xFFFFFFFF);
								
								//等待管理员处理
        }
    }
				//延时10ms，给其他任务执行的时间
				vTaskDelay(10);
	}
}

