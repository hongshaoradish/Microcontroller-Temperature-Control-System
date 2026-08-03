#include "../inc/buzzer.h"
#include "../inc/delay.h"

// ==================== 引脚定义 ====================
sbit BEEP = P1^5;

// ==================== 微秒延时（仅用于短鸣，不阻塞中断） ====================
void Buzzer_DelayUs(unsigned int t)
{
    while (t--);
}

// ==================== 初始化 ====================
void BUZZER_Init(void)
{
    BEEP = 1;
}

// ==================== 按键反馈：短鸣（阻塞式，但很短） ====================
void BUZZER_ShortBeep(void)
{
    unsigned int i;
    
    for (i = 0; i < 375; i++)
    {
        BEEP = 0;
        Buzzer_DelayUs(80);
        BEEP = 1;
        Buzzer_DelayUs(80);
    }
}

// ==================== 非阻塞报警状态机 ====================
static unsigned char alarm_active = 0;
static unsigned char alarm_repeat = 0;
static unsigned char alarm_phase = 0;
static unsigned int alarm_on_time = 0;
static unsigned int alarm_off_time = 0;
static unsigned int alarm_counter = 0;
static unsigned int alarm_pwm_count = 0;

void BUZZER_AlarmStart(unsigned int on_time_ms, unsigned int off_time_ms, unsigned char repeat)
{
    if (alarm_active)
        BUZZER_AlarmStop();
    
    alarm_on_time = on_time_ms;
    alarm_off_time = off_time_ms;
    alarm_repeat = repeat;
    alarm_phase = 0;
    alarm_counter = 0;
    alarm_pwm_count = 0;
    alarm_active = 1;
}

void BUZZER_AlarmStop(void)
{
    alarm_active = 0;
    alarm_repeat = 0;
    BEEP = 1;
}

unsigned char BUZZER_IsAlarmActive(void)
{
    return alarm_active;
}

// ==================== 中断中调用（每 2ms，只做计时，不输出 PWM） ====================
void BUZZER_AlarmProcess(void)
{
    if (!alarm_active)
        return;
    
    alarm_counter += 2;
    
    if (alarm_phase == 0)
    {
        if (alarm_counter >= alarm_on_time)
        {
            alarm_counter = 0;
            alarm_phase = 1;
            alarm_pwm_count = 0;
            BEEP = 1;
        }
    }
    else
    {
        if (alarm_counter >= alarm_off_time)
        {
            alarm_counter = 0;
            
            if (alarm_repeat > 0)
            {
                alarm_repeat--;
                if (alarm_repeat == 0)
                {
                    alarm_active = 0;
                    BEEP = 1;
                    return;
                }
            }
            
            alarm_phase = 0;
            alarm_pwm_count = 0;
            BEEP = 0;
        }
    }
}

// ==================== 主循环中调用（生成 PWM 方波） ====================
void BUZZER_PWM_Output(void)
{
    static unsigned char pwm_state = 0;
    static unsigned int pwm_counter = 0;
    
    if (!alarm_active)
        return;
    
    if (alarm_phase == 0)
    {
        pwm_counter++;
        if (pwm_counter >= 1)
        {
            pwm_counter = 0;
            pwm_state = !pwm_state;
            
            if (pwm_state)
                BEEP = 0;
            else
                BEEP = 1;
        }
    }
}

// ==================== 兼容接口 ====================
void BUZZER_On(void)
{
    BUZZER_ShortBeep();
}

void BUZZER_Off(void)
{
    BEEP = 1;
}

void BUZZER_Toggle(void)
{
    BUZZER_ShortBeep();
}

// ==================== 阻塞式报警（已废弃，保留空函数避免链接错误） ====================
void BUZZER_Alarm(unsigned int on_time_ms, unsigned int off_time_ms, unsigned char repeat)
{
    (void)on_time_ms;
    (void)off_time_ms;
    (void)repeat;
    // 不再使用阻塞式报警
}