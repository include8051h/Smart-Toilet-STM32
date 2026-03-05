 /******************************************************************************
 * @file    OLED.h
 * @author  林子钦
 * @email   2350768134@qq.com
 * @date    2026/01/13
 * @brief   OLED显示功能具体实现代码头文件
 * @version 无
 * @note    无
 ******************************************************************************/



#ifndef __OLED_H
#define __OLED_H

enum state_oled
{
	NOTHING = 1,
	DONE_STANDING_WATER,
	DONE_FREE,
	DONE_USING,
	DONE_DANGER
}typedef state_oled;




/**
 * @func			 	OLED_Init
 * @brief      	OLED初始化函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void OLED_Init(void);



/**
 * @func			 	OLED_Clear
 * @brief      	OLED清屏函数
 * @param		  	无
 * @retval     	无
 * @note       	无
 */
void OLED_Clear(void);



/**
 * @func			 	OLED_ShowChar
 * @brief      	OLED显示字符函数
 * @param		  	参数一：uint8_t（横坐标）
								参数二：uint8_t（纵坐标）
								参数三：char（要显示的字符）
 * @retval     	int
 * @note       	无
 */
int OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);



/**
 * @func			 	OLED_ShowChar
 * @brief      	OLED显示字符函数
 * @param		  	参数一：uint8_t（横坐标）
								参数二：uint8_t（纵坐标）
								参数三：char（要显示的字符）
 * @retval     	int
 * @note       	无
 */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);



/**
 * @func			 	OLED_ShowNum
 * @brief      	OLED显示十进制数字函数
 * @param		  	参数一：uint8_t（横坐标）
								参数二：uint8_t（纵坐标）
								参数三：uint32_t（要显示的数字）
								参数四：uint8_t(要显示的数字的长度)
 * @retval     	无
 * @note       	无
 */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);



/**
 * @func			 	OLED_ShowSignedNum
 * @brief      	OLED显示带符号的十进制数字函数
 * @param		  	参数一：uint8_t（横坐标）
								参数二：uint8_t（纵坐标）
								参数三：uint32_t（要显示的数字）
								参数四：uint8_t(要显示的数字的长度)
 * @retval     	无
 * @note       	无
 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);



/**
 * @func			 	OLED_ShowHexNum
 * @brief      	OLED显示十六进制数字函数
 * @param		  	参数一：uint8_t（横坐标）
								参数二：uint8_t（纵坐标）
								参数三：uint32_t（要显示的数字）
								参数四：uint8_t(要显示的数字的长度)
 * @retval     	无
 * @note       	无
 */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);



/**
 * @func			 	OLED_ShowBinNum
 * @brief      	OLED显示二进制数字函数
 * @param		  	参数一：uint8_t（横坐标）
								参数二：uint8_t（纵坐标）
								参数三：uint32_t（要显示的数字）
								参数四：uint8_t(要显示的数字的长度)
 * @retval     	无
 * @note       	无
 */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);



/**
 * @func			 	OLED_ShowChinese
 * @brief      	OLED显示汉字函数
 * @param		  	参数一：uint8_t（横坐标）
								参数二：uint8_t（纵坐标）
								参数三：uint8_t（要显示的汉字在字模数组中的序号）
 * @retval     	无
 * @note       	无
 */
void OLED_ShowChinese(uint8_t Line, uint8_t Column,uint8_t num);
void OLED_BMP(int i);
#endif
