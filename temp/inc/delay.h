#ifndef __DELAY_H__
#define __DELAY_H__

#include <reg52.h>
#include <intrins.h>

// ==================== 通用延时 ====================
void Delay_Us(unsigned int us);
void Delay_Ms(unsigned int ms);

// ==================== DS18B20 专用精确延时 ====================
void Delay_2us(void);
void Delay_5us(void);
void Delay_15us(void);
void Delay_60us(void);
void Delay_480us(void);
void Delay_550us(void);

#endif