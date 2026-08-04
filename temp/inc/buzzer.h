#ifndef __BUZZER_H__
#define __BUZZER_H__

#include <reg52.h>

// ==================== 函数声明 ====================
void BUZZER_Init(void);

// 按键反馈：短鸣（约 60ms）
void BUZZER_ShortBeep(void);

// 兼容接口
void BUZZER_On(void);
void BUZZER_Off(void);
void BUZZER_Toggle(void);

// 非阻塞报警（用于温度报警）
void BUZZER_AlarmStart(unsigned int on_time_ms, unsigned int off_time_ms, unsigned char repeat);
void BUZZER_AlarmStop(void);
unsigned char BUZZER_IsAlarmActive(void);

// 中断中调用（每 2ms，只做计时）
void BUZZER_AlarmProcess(void);

// 主循环中调用（生成 PWM 方波）
void BUZZER_PWM_Output(void);

// 阻塞式报警（备用）
void BUZZER_Alarm(unsigned int on_time_ms, unsigned int off_time_ms, unsigned char repeat);

#endif