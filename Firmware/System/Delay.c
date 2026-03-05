#include "Delay.h"

// TIM1 初始化标志
static uint8_t tim1_initialized = 0;

/**
  * @brief  初始化 TIM1 用于延时
  * @param  无
  * @retval 无
  */
void Delay_Init(void)
{
    if(tim1_initialized) return;
    
    // 开启 TIM1 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    
    // 定时器配置：1us 计数
    // 72MHz / (71+1) = 1MHz，每个计数 1us
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;          // 最大计数值
    TIM_TimeBaseStructure.TIM_Prescaler = 71;           // 预分频器
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    
    tim1_initialized = 1;
}

/**
  * @brief  微秒级延时（使用 TIM1）
  * @param  us 延时时长，范围：0~65535
  * @retval 无
  */
void Delay_us(uint32_t us)
{
    if(!tim1_initialized) {
        Delay_Init();
    }
    
    TIM_SetCounter(TIM1, 0);
    TIM_Cmd(TIM1, ENABLE);
    
    while(TIM_GetCounter(TIM1) < us);
    
    TIM_Cmd(TIM1, DISABLE);
}

/**
  * @brief  毫秒级延时
  * @param  ms 延时时长
  * @retval 无
  */
void Delay_ms(uint32_t ms)
{
    // 对于较长的延时，使用循环调用微秒延时
    while(ms--)
    {
        Delay_us(1000);
    }
}

/**
  * @brief  秒级延时
  * @param  s 延时时长
  * @retval 无
  */
void Delay_s(uint32_t s)
{
    while(s--)
    {
        Delay_ms(1000);
    }
}
