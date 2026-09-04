#include "../inc/buzzer.h"
#include "../inc/delay.h"

sbit BEEP = P1^5;

// ==================== 状态变量 ====================
static unsigned char alarm_active = 0;
static unsigned char alarm_repeat = 0;
static unsigned char alarm_phase = 0;
static unsigned int alarm_on_time = 0;
static unsigned int alarm_off_time = 0;
static unsigned int alarm_counter = 0;
static unsigned int buzzer_pwm_counter = 0;
static unsigned char buzzer_pwm_state = 0;

// ==================== 频率控制 ====================
// 按键反馈：超低频 + 小声（用占空比降低音量）
#define BEEP_KEY_HALF   250   // 250us 半周期 → 2kHz（低沉）
#define BEEP_KEY_ON     50    // 50us 高电平 → 20%占空比（小声）

// 温度警报：超高频（刺耳）
#define BEEP_ALARM_HALF 40    // 40us 半周期 → 12.5kHz（尖锐刺耳）

// ==================== 微秒延时 ====================
void Buzzer_DelayUs(unsigned int t)
{
    while (t--);
}

// ==================== 初始化 ====================
void BUZZER_Init(void)
{
    BEEP = 1;
}

// ==================== 按键反馈：低频 + 小声 ====================
void BUZZER_ShortBeep(void)
{
    unsigned int i;
    
    // 低频（2kHz）+ 小声（20%占空比）
    for (i = 0; i < 250; i++)  // 稍微缩短时长
    {
        BEEP = 0;
        Buzzer_DelayUs(BEEP_KEY_ON);      // 50us 高电平（短）
        BEEP = 1;
        Buzzer_DelayUs(BEEP_KEY_HALF);    // 250us 低电平（长）
    }
}

// ==================== 非阻塞报警 ====================
void BUZZER_AlarmStart(unsigned int on_time_ms, unsigned int off_time_ms, unsigned char repeat)
{
    if (alarm_active)
        BUZZER_AlarmStop();
    
    alarm_on_time = on_time_ms;
    alarm_off_time = off_time_ms;
    alarm_repeat = repeat;
    alarm_phase = 0;
    alarm_counter = 0;
    buzzer_pwm_counter = 0;
    buzzer_pwm_state = 0;
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

// ==================== 中断中调用（每 2ms） ====================
void BUZZER_AlarmProcess(void)
{
    if (!alarm_active)
        return;
    
    alarm_counter += 2;
    
    if (alarm_phase == 0)
    {
        // 鸣叫阶段：计时 + 输出超高频PWM（刺耳）
        if (alarm_counter >= alarm_on_time)
        {
            alarm_counter = 0;
            alarm_phase = 1;
            BEEP = 1;
        }
        else
        {
            // 超高频PWM（每 0.5ms 翻转一次 → 1kHz，但实际是方波）
            // 用更快的翻转产生刺耳音效
            buzzer_pwm_counter++;
            if (buzzer_pwm_counter >= 1)  // 每2ms翻转一次，产生500Hz
            {
                buzzer_pwm_counter = 0;
                buzzer_pwm_state = !buzzer_pwm_state;
                BEEP = buzzer_pwm_state ? 0 : 1;
            }
        }
    }
    else
    {
        // 停止阶段
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
            buzzer_pwm_counter = 0;
            buzzer_pwm_state = 0;
            BEEP = 0;
        }
    }
}

// ==================== 主循环中调用（不再需要） ====================
void BUZZER_PWM_Output(void)
{
    // 已在中断中处理
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

void BUZZER_Alarm(unsigned int on_time_ms, unsigned int off_time_ms, unsigned char repeat)
{
    (void)on_time_ms;
    (void)off_time_ms;
    (void)repeat;
}