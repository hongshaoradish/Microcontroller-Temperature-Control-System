#ifndef __FAN_H__
#define __FAN_H__

#include <reg52.h>

// ==================== 引脚定义 ====================
sbit FAN = P1^0;

// ==================== 函数声明 ====================
void FAN_Init(void);
void FAN_On(void);
void FAN_Off(void);
void FAN_Control(unsigned char enable);
void FAN_Tick(void);        // ✅ 新增：定时器中断中调用

#endif