#include "help.h"

uint8_t level = 0;	//用于存储电平的全局变量

QueueHandle_t xKeyEventQueue = NULL;	//声明一个消息队列句柄，用来传递按键的电平值



//求生按键初始化函数
void Key_EXTI_Init(void)
{
    // 使能 GPIOB 和 AFIO 时钟，AFIO用于EXTI映射
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    //配置 PB5 为下拉输入，无按键时为低，按下为高
    GPIO_InitTypeDef GPIO_InitStruct;	//定义一个GPIO初始化结构体，用于存储各种配置参数
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;	//选中5号引脚
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD; //模式选择下拉输入 Internal Pull-Down
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;	//速度选择50MHz
    GPIO_Init(GPIOB, &GPIO_InitStruct);	//初始化上述配置参数

    //将PB5映射到EXTI5
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);

    //配置EXTI5上升沿触发，检测按下动作
    EXTI_InitTypeDef EXTI_InitStruct;	//定义一个EXTI初始化结构体
    EXTI_InitStruct.EXTI_Line = EXTI_Line5;		//指定要配置的 EXTI 中断线为 Line 5
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;	//设置EXTI的工作模式为中断模式
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //双边沿触发
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;	//使能指定的EXTI 线
    EXTI_Init(&EXTI_InitStruct);

    // 5. 配置 NVIC：使能 EXTI9_5_IRQn 中断通道
    NVIC_InitTypeDef NVIC_InitStruct;	//定义一个NVIC初始化结构体
    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;	//指定要配置的中断通道为 EXTI9_5_IRQn
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;	//设置该中断的抢占优先级为 0
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;	//设置子优先级为1
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;	//使能中断通道
    NVIC_Init(&NVIC_InitStruct);
		
		xKeyEventQueue = xQueueCreate(5, sizeof(uint8_t));	//创建信息队列
}



//KeyState_t get_key_state(void)
//{
//	return key_state;
//}
//获取按键的电平
uint8_t get_level(void)
{
	return level;	//返回电平值
}


//外部中断服务函数
void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line5) != RESET)//如果中断标志位不为0
    {
				//读取按键引脚电平
         uint8_t level = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) ? 1 : 0;
        
        // 发送电平事件到队列（非阻塞）
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(xKeyEventQueue, &level, &xHigherPriorityTaskWoken);
        
        EXTI_ClearITPendingBit(EXTI_Line5);	//清除中断标志位
			
				//如果 xHigherPriorityTaskWoken == pdTRUE，则立即触发一次任务调度
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
