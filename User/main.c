 /******************************************************************************
 * @file    main.c
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/07
 * @brief   作为整个程序运行的入口代码文件
 * @version 无
 * @note    无
 ******************************************************************************/

#include "FreeRTOS_demo.h"


/**
 * @func			 	main
 * @brief      	整个程序运行的入口函数
 * @param		  	无
 * @retval     	int
 * @note       	无
 */
 int main(void)
 {	
	 //抢占优先级和响应优先各占2位
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	 
	 //启动FreeRTOS功能
	 FreeRTOS_Start();
}
