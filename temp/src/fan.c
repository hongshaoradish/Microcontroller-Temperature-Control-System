#include "../inc/fan.h"

static unsigned char fan_running = 0;
static unsigned char last_fan_state = 0xFF;  // 记录上次状态

void FAN_Init(void)
{
    FAN = 0;
    fan_running = 0;
    last_fan_state = 0xFF;
}

void FAN_Control(unsigned char enable)
{
    unsigned char target_state;
    
    target_state = enable ? 1 : 0;
    
    // ✅ 只有在状态真正变化时才操作引脚
    if (target_state != last_fan_state)
    {
        last_fan_state = target_state;
        
        if (target_state)
        {
            FAN = 1;
            fan_running = 1;
        }
        else
        {
            FAN = 0;
            fan_running = 0;
        }
    }
}