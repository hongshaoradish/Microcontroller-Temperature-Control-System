#include "../inc/lcd1602.h"
#include "../inc/delay.h"

// ============================================
// LCD 写指令
// ============================================
void LCD_WriteCmd(unsigned char cmd)
{
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_DB = cmd;
    
    LCD_E = 1;
    Delay_Us(1);
    LCD_E = 0;
    Delay_Us(20);
}

// ============================================
// LCD 写数据
// ============================================
void LCD_WriteData(unsigned char dat)
{
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_DB = dat;
    
    LCD_E = 1;
    Delay_Us(1);
    LCD_E = 0;
    Delay_Us(20);
}

// ============================================
// LCD 写字符串
// ============================================
void LCD_WriteString(unsigned char row, unsigned char *str)
{
    LCD_SetCursor(row, 0);
    
    while (*str != '\0')
    {
        LCD_RS = 1;
        LCD_RW = 0;
        LCD_DB = *str++;
        
        LCD_E = 1;
        Delay_Us(1);
        LCD_E = 0;
        Delay_Us(10);
    }
}

// ============================================
// LCD 初始化
// ============================================
void LCD_Init(void)
{
    Delay_Ms(15);
    
    LCD_RS = 0;
    LCD_RW = 0;
    
    LCD_DB = 0x30;
    LCD_E = 1;
    Delay_Us(5);
    LCD_E = 0;
    Delay_Ms(5);
    
    LCD_DB = 0x30;
    LCD_E = 1;
    Delay_Us(5);
    LCD_E = 0;
    Delay_Us(200);
    
    LCD_DB = 0x30;
    LCD_E = 1;
    Delay_Us(5);
    LCD_E = 0;
    Delay_Us(200);
    
    LCD_WriteCmd(0x38);
    LCD_WriteCmd(0x0C);
    LCD_WriteCmd(0x01);
    Delay_Ms(2);
    LCD_WriteCmd(0x06);
}

// ============================================
// LCD 清屏
// ============================================
void LCD_Clear(void)
{
    LCD_WriteCmd(0x01);
    Delay_Ms(2);
}

// ============================================
// LCD 设置光标
// ============================================
void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char addr;
    
    if (row == 0)
        addr = 0x00 + col;
    else
        addr = 0x40 + col;
    
    LCD_WriteCmd(0x80 | addr);
}