 /******************************************************************************
 * @file    Safety.c
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/09
 * @brief   卫生间使用时间定时器具体实现代码源文件
 * @version 无
 * @note    无
 ******************************************************************************/



#include "Safety.h"

// 全局变量定义
volatile uint32_t safety_timer_counter = 0;//安全事故时钟计数值
volatile uint8_t safety_timer_enabled = 0;//安全事故时钟启动标志
volatile uint8_t safety_target_minutes = 0;//安全事故目标计时时间
volatile uint16_t safety_accident_flag = 0;//安全事故发生标志

uint8_t flag_accident = 0;

/*用volatile修饰全局变量，因为这些变量可能在中断服务程序中被修改，
或者在主循环和中断服务程序中被访问。使用volatile可以确保每次访问变量时都从内存中读取，
而不是使用寄存器中的缓存值，从而保证数据的实时性。*/

/**
 * @func			 	safety_timer_init
 * @brief      	卫生间使用时间定时器初始化函数
 * @param		  	参数一： uint16_t 预设时间
 * @retval     	无
 * @note       	无
 */
void safety_timer_init(uint16_t target_minutes)
{
    // 限制输入范围
    if (target_minutes < 1) target_minutes = 1;//最小计时1分钟
    if (target_minutes > 60*60) target_minutes = 60*60;//最大计时60分钟
    
    safety_target_minutes = target_minutes;	//设置预设时间
    safety_timer_counter = 0;	//计数值
    safety_timer_enabled = 0;	//定时器使能标志位
    safety_accident_flag = 0;	//超时标志位
    
    // 开启TIM3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    // 定时器配置
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;	//定义一个时基单元初始化结构体，用于存储各种初始化参数
    TIM_TimeBaseStructure.TIM_Period = 9999;           // 自动重装载值
    TIM_TimeBaseStructure.TIM_Prescaler = 7199;        // 预分频值 
    // 72MHz / (7199+1) = 10kHz, 10kHz / 10000 = 1Hz (1秒)
    TIM_TimeBaseStructure.TIM_ClockDivision = 0; //不分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);	//初始化上述配置参数
    
    // 使能定时器中断
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    // 配置NVIC
    NVIC_InitTypeDef NVIC_InitStructure;	//定义一个NVIC初始化结构体，用于存储各种参数
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	//选择中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//抢占优先级为2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;//子优先级为3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能中断
    NVIC_Init(&NVIC_InitStructure);//将上述配置参数进行初始化
    
    // 启动定时器，但不立即开始计数
    TIM_Cmd(TIM3, ENABLE);
}

//启动卫生间使用时间定时器
void safety_timer_start(void)
{
    safety_timer_counter = 0;	//计数值
    safety_timer_enabled = 1;	//定时器使能标志位
    safety_accident_flag = 0;	//定时器超时标志位
}

//停止定时器
void safety_timer_stop(void)
{
    safety_timer_enabled = 0;//定时器使能标志位赋值为0，表示停止
}

//重置定时器
void safety_timer_reset(void)
{
    safety_timer_counter = 0;//计数值清零
    safety_accident_flag = 0;//重置超时标志位
}

//更改预设时间
void safety_timer_set_target(uint16_t target_minutes)
{
    if (target_minutes < 1) target_minutes = 1;	//如果预设时间小于1秒就将其设为1秒
    if (target_minutes > 60*60) target_minutes = 60*60;	//如果预设时间大于1小时就将其设为1小时
    
    safety_target_minutes = target_minutes;	//应用修改后的预设时间
    safety_timer_reset();	//重置定时器
}



//获取当前计时状态（秒）
uint32_t safety_timer_get_elapsed_sec(void)
{
    return safety_timer_counter%60;//返回已计时秒数
}



//获取当前计时状态（分钟）
uint32_t safety_timer_get_elapsed_min(void)
{
    return safety_timer_counter/60;//返回已计时分钟数
}

/**
  * @brief  获取剩余时间
  * @param  None
  * @retval 剩余秒数
  */
/*uint32_t safety_timer_get_remaining(void)
{
    uint32_t target_seconds = safety_target_minutes * 60;
    if (safety_timer_counter >= target_seconds) {
        return 0;
    }
    return target_seconds - safety_timer_counter;
}*/



//获取超时标志位的值，1:发生安全事故, 0:正常
uint8_t safety_timer_get_accident_flag(void)
{
	switch(safety_accident_flag)//利用switch循环判断该返回何值
	{
		case 1://如果全局超时标志位为1，表示超时
			flag_accident = 1;//那就返回1
		break;//跳出switch循环
		
		case 0://如果全局超时标志位为0，表示未超时
			flag_accident = 0;//那就返回0
		break;//跳出switch循环
	}
    return flag_accident;//返回超时标志位的值
}

//清除超时标志位
void safety_timer_clear_accident_flag(void)
{
    safety_accident_flag = 0;//将全局超时标志位置为0
}

//TIM3中断服务函数
void TIM3_IRQHandler(void)
{
		//如果中断标志位触发
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);//清除中断标志位
        
        // 如果计时器使能，则计数
        if (safety_timer_enabled) {
            safety_timer_counter++;//计数一次
            
            // 如果超时
            if (safety_timer_counter >= (safety_target_minutes)) {
                safety_accident_flag = 1;	//超时标志位置一，表示超时
								safety_timer_stop();	//停止定时器
								//safety_timer_reset();
            }
        }
    }
}


//void display_safety_timer_info(void)
//{
//    //char buffer[20];
//    
//    // 获取计时信息
//    uint32_t elapsed_seconds = safety_timer_get_elapsed();
//    //uint32_t remaining_seconds = safety_timer_get_remaining();
//    uint16_t accident_flag = safety_timer_get_accident_flag();
//    
//    // 计算分钟和秒
//    elapsed_minutes = elapsed_seconds / 60;
//    elapsed_secs = elapsed_seconds % 60;
//    
//    //uint32_t remaining_minutes = remaining_seconds / 60;
//    //uint32_t remaining_secs = remaining_seconds % 60;
//    
//    // 显示已计时时间
//    OLED_ShowString(2, 1, "Elapsed:        ");
//    OLED_ShowNum(2, 9, elapsed_minutes, 2);
//    OLED_ShowString(2, 11, "m");
//    OLED_ShowNum(2, 12, elapsed_secs, 2);
//    OLED_ShowString(2, 14, "s");
//    
//    // 显示剩余时间
//    /*OLED_ShowString(2, 1, "Remain:         ");
//    OLED_ShowNum(2, 9, remaining_minutes, 2);
//    OLED_ShowString(2, 11, "m");
//    OLED_ShowNum(2, 12, remaining_secs, 2);
//    OLED_ShowString(2, 14, "s");*/
//    
//    // 显示事故标志
//    OLED_ShowString(3, 1, "Accident Flag:  ");
//    OLED_ShowNum(3, 16, accident_flag, 1);
//    
//    // 显示进度条
//    /*if (!accident_flag && safety_target_minutes > 0) {
//        uint8_t progress = (elapsed_seconds * 100) / (safety_target_minutes * 60);
//        if (progress > 100) progress = 100;
//        
//        OLED_ShowString(4, 1, "Progress:       ");
//        OLED_ShowNum(4, 11, progress, 3);
//        OLED_ShowString(4, 14, "% ");
//    }*/
//}
