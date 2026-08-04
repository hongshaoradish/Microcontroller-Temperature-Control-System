#ifndef __KEY_H__
#define __KEY_H__

#include <reg52.h>

// ==================== 按键引脚定义 ====================
// 物理线路接反，软件交换
sbit KEY_INC  = P3^1;   // 增加温度
sbit KEY_DEC  = P3^0;   // 减少温度
sbit KEY_SET  = P3^2;   // 修改模式切换
sbit KEY_VIEW = P3^3;   // 切换调节目标

// ==================== 函数声明 ====================
void KEY_Init(void);

// 非阻塞按键扫描（在定时器中断中调用）
void KEY_Scan(void);

// 获取按键状态（主循环调用）
unsigned char KEY_GetPress(void);

#endif