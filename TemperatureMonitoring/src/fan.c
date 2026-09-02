#include "../inc/fan.h"
#include "../inc/delay.h"

static unsigned char fan_running = 0;
static unsigned char fan_target = 0;      // 目标状态
static unsigned int fan_delay_counter = 0; // 延时计数器

void FAN_Init(void)
{
    FAN = 0;
    fan_running = 0;
    fan_target = 0;
    fan_delay_counter = 0;
}

void FAN_On(void)
{
    fan_target = 1;
    fan_delay_counter = 0;   // 重置延时计数器
}

void FAN_Off(void)
{
    fan_target = 0;
    fan_delay_counter = 0;
}

// ==================== 核心：在主循环中调用 ====================
void FAN_Control(unsigned char enable)
{
    if (enable)
    {
        FAN_On();
    }
    else
    {
        FAN_Off();
    }
}

// ==================== 在定时器中断中调用（每 2ms） ====================
void FAN_Tick(void)
{
    // 如果目标状态和当前状态相同，不做任何事
    if (fan_target == fan_running)
        return;
    
    // 延时计数器累加
    fan_delay_counter++;
    
    // 等待 100ms 后才执行（让 LCD 有足够时间完成操作）
    if (fan_delay_counter < 50)  // 50 * 2ms = 100ms
        return;
    
    // 执行状态切换
    fan_delay_counter = 0;
    fan_running = fan_target;
    
    if (fan_running)
    {
        FAN = 1;
    }
    else
    {
        FAN = 0;
    }
}