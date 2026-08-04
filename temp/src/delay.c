#include "../inc/delay.h"
#include <intrins.h>

// ============================================
// 12MHz 晶振精确延时函数
// 基于 12MHz 晶振：1个机器周期 = 1us
// ============================================

// ----- 微秒延时（通用）-----
void Delay_Us(unsigned int us)
{
    unsigned int i;
    while (us--) {
        i = 2;
        while (i--);
    }
}

// ----- 毫秒延时（通用）-----
void Delay_Ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}

// ============================================
// DS18B20 专用精确延时
// ============================================

// ----- 2us 延时 -----
void Delay_2us(void)
{
    _nop_(); _nop_();
}

// ----- 5us 延时 -----
void Delay_5us(void)
{
    _nop_(); _nop_(); _nop_(); _nop_();
    _nop_();
}

// ----- 15us 延时 -----
void Delay_15us(void)
{
    unsigned char i = 7;
    while (--i) {
        _nop_();
    }
}

// ----- 60us 延时 -----
void Delay_60us(void)
{
    unsigned char i = 27;
    while (--i) {
        _nop_(); _nop_();
    }
}

// ----- 480us 延时 -----
void Delay_480us(void)
{
    unsigned int i = 235;
    while (--i) {
        _nop_();
    }
}

// ----- 550us 延时 -----
void Delay_550us(void)
{
    unsigned int i = 270;
    while (--i) {
        _nop_();
    }
}