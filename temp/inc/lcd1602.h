#ifndef __LCD1602_H__
#define __LCD1602_H__

#include <reg52.h>
#include "../inc/delay.h"

// ==================== 引脚定义 ====================
sbit LCD_RS = P2^6;
sbit LCD_RW = P2^5;
sbit LCD_E  = P2^7;
#define LCD_DB P0

// ==================== LCD 忙标志（供外部使用） ====================
extern bit lcd_busy;

// ==================== 函数声明 ====================
void LCD_Init(void);
void LCD_WriteCmd(unsigned char cmd);
void LCD_WriteData(unsigned char dat);
void LCD_WriteString(unsigned char row, unsigned char *str);
void LCD_Clear(void);
void LCD_SetCursor(unsigned char row, unsigned char col);
unsigned char LCD_IsBusy(void);

#endif