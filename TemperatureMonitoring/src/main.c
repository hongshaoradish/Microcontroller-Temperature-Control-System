#include <reg52.h>
#include "../inc/delay.h"
#include "../inc/key.h"
#include "../inc/eeprom.h"
#include "../inc/ds18b20.h"
#include "../inc/lcd1602.h"
#include "../inc/buzzer.h"
#include "../inc/fan.h"


// ==================== 温度变量（×10格式） ====================
unsigned int high_temp = 380;
unsigned int low_temp = 150;
unsigned int real_temp = 200;

// ==================== 系统状态 ====================
unsigned char modify_mode = 0;      // 0=正常显示，1=修改模式
unsigned char adjust_target = 0;    // 0=调上限(H)，1=调下限(L)

// ==================== 闪烁控制 ====================
unsigned int blink_counter = 0;
unsigned char blink_state = 1;

// ==================== 温度读取控制 ====================
unsigned int read_counter = 0;
unsigned char temp_convert_waiting = 0;
unsigned int temp_convert_timer = 0;

// ==================== 传感器状态 ====================
unsigned char sensor_ok = 0;
unsigned char read_fail_count = 0;
unsigned char sensor_checked = 0;

// ==================== 模拟温度控制 ====================
unsigned int sim_temp = 200;
unsigned char sim_direction = 1;
unsigned char use_sim_temp = 1;

// ==================== 报警状态 ====================
unsigned char alarm_state = 0;
unsigned char buzzer_alarm_active = 0;

// ==================== 启动画面状态 ====================
unsigned char startup_phase = 1;

// ==================== 函数声明 ====================
void Timer0_Init(void);
void LoadThresholdFromEEPROM(void);
void SaveThresholdToEEPROM(void);
void AdjustTemperature(unsigned char direction);
void GenerateSimulatedTemperature(void);
void MonitorTemperature(void);
void UpdateLCD(void);
void CheckBuzzerAlarm(void);
void ProcessTemperatureRead(void);
void ShowStartupInfo(void);
void FAN_Control(unsigned char enable);

// ==================== 定时器0初始化 ====================
void Timer0_Init(void)
{
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TH0 = 0xF8;
    TL0 = 0x30;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

// ==================== 定时器0中断 ====================
void Timer0_ISR(void) interrupt 1
{
    TH0 = 0xF8;
    TL0 = 0x30;
    
    KEY_Scan();
    
    blink_counter++;
    if (blink_counter >= 250)
    {
        blink_counter = 0;
        blink_state = !blink_state;
    }
    
    if (temp_convert_waiting)
    {
        temp_convert_timer += 2;
        if (temp_convert_timer >= 750)
        {
            temp_convert_waiting = 0;
            temp_convert_timer = 0;
        }
    }
    
    BUZZER_AlarmProcess();
}

// ==================== EEPROM 加载阈值 ====================
void LoadThresholdFromEEPROM(void)
{
    unsigned int lower, upper;
    
    if (EEPROM_ReadThreshold(&lower, &upper))
    {
        low_temp = lower;
        high_temp = upper;
    }
    else
    {
        low_temp = 150;
        high_temp = 380;
        EEPROM_SaveThreshold(low_temp, high_temp);
    }
    
    if (high_temp > 999) high_temp = 999;
    if (low_temp > 999) low_temp = 999;
    if (low_temp > high_temp) low_temp = high_temp;
}

// ==================== EEPROM 保存阈值 ====================
void SaveThresholdToEEPROM(void)
{
    if (high_temp > 999) high_temp = 999;
    if (low_temp > 999) low_temp = 999;
    if (low_temp > high_temp) low_temp = high_temp;
    
    EEPROM_SaveThreshold(low_temp, high_temp);
}

// ==================== 温度调节 ====================
void AdjustTemperature(unsigned char direction)
{
    if (adjust_target == 0)
    {
        if (direction == 0)
        {
            if (high_temp < 999)
                high_temp += 5;
            if (high_temp < low_temp)
                high_temp = low_temp;
        }
        else
        {
            if (high_temp > low_temp)
                high_temp -= 5;
        }
    }
    else
    {
        if (direction == 0)
        {
            if (low_temp < high_temp)
                low_temp += 5;
            if (low_temp > high_temp)
                high_temp = low_temp;
        }
        else
        {
            if (low_temp > 0)
                low_temp -= 5;
            if (low_temp > high_temp)
                low_temp = high_temp;
        }
    }
    
    if (high_temp > 999) high_temp = 999;
    if (low_temp > 999) low_temp = 999;
    if (low_temp > high_temp) low_temp = high_temp;
}

// ==================== 模拟温度生成 ====================
void GenerateSimulatedTemperature(void)
{
    if (sim_direction == 1)
    {
        sim_temp += 2;
        if (sim_temp >= 400)
        {
            sim_temp = 400;
            sim_direction = 0;
        }
    }
    else
    {
        if (sim_temp > 200)
        {
            sim_temp -= 2;
        }
        else
        {
            sim_temp = 200;
            sim_direction = 1;
        }
    }
    real_temp = sim_temp;
}

// ==================== 温度监控与报警 ====================
void MonitorTemperature(void)
{
    if (real_temp >= high_temp)
    {
        alarm_state = 1;
        if (!BUZZER_IsAlarmActive())
        {
            buzzer_alarm_active = 1;
            BUZZER_AlarmStart(500, 300, 10);
        }
    }
    else if (real_temp <= low_temp)
    {
        alarm_state = 2;
        if (!BUZZER_IsAlarmActive())
        {
            buzzer_alarm_active = 1;
            BUZZER_AlarmStart(400, 400, 8);
        }
    }
    else
    {
        alarm_state = 0;
        if (BUZZER_IsAlarmActive())
        {
            buzzer_alarm_active = 0;
            BUZZER_AlarmStop();
        }
    }
}

// ==================== 传感器检测 ====================
void CheckSensor(void)
{
    unsigned char result;
    
    result = DS18B20_Init();
    
    if (result)
    {
        sensor_ok = 1;
        use_sim_temp = 0;
    }
    else
    {
        sensor_ok = 0;
        use_sim_temp = 1;
    }
    sensor_checked = 1;
}

// ==================== 非阻塞温度读取 ====================
void ProcessTemperatureRead(void)
{
    unsigned int temp_result;
    
    if (!sensor_ok || use_sim_temp)
    {
        GenerateSimulatedTemperature();
        return;
    }
    
    temp_result = DS18B20_ReadTemp();
    
    if (temp_result <= 999)
    {
        real_temp = temp_result;
        read_fail_count = 0;
        if (use_sim_temp)
        {
            use_sim_temp = 0;
        }
    }
    else
    {
        read_fail_count++;
        
        // 显示错误信息（前3次显示，之后不再刷屏）
        if (read_fail_count <= 3)
        {
            LCD_WriteString(0, "Read Err:       ");
            if (temp_result == 0xFFFF)
                LCD_WriteString(1, "RESET FAIL      ");
            else if (temp_result == 0xFFFE)
                LCD_WriteString(1, "CRC ERR         ");
            else
                LCD_WriteString(1, "UNKNOWN ERR     ");
            Delay_Ms(300);
            UpdateLCD();
        }
        
        if (read_fail_count > 10)
        {
            sensor_ok = 0;
            use_sim_temp = 1;
            read_fail_count = 0;
            LCD_WriteString(0, "Sensor FAIL     ");
            LCD_WriteString(1, "Use SIM temp    ");
            Delay_Ms(1000);
        }
    }
}

// ==================== 启动信息显示 ====================
void ShowStartupInfo(void)
{
    if (sensor_ok)
    {
        LCD_WriteString(0, "Sensor: OK      ");
    }
    else
    {
        LCD_WriteString(0, "Sensor: FAIL    ");
    }
    
    if (use_sim_temp)
    {
        LCD_WriteString(1, "Mode: SIMULATE  ");
    }
    else
    {
        LCD_WriteString(1, "Mode: REAL      ");
    }
    
    Delay_Ms(2000);
    startup_phase = 0;
    LCD_Clear();
}

// ==================== LCD 更新 ====================
void UpdateLCD(void)
{
    unsigned char temp_str[18];
    unsigned char h_h, h_t, h_o;
    unsigned char l_h, l_t, l_o;
    unsigned char int_part, frac_part;
    unsigned char i;
    
    // ===== 第一行：实时温度 + 状态 =====
    int_part = real_temp / 10;
    frac_part = real_temp % 10;
    
    temp_str[0] = 'T';
    temp_str[1] = 'e';
    temp_str[2] = 'm';
    temp_str[3] = 'p';
    temp_str[4] = ':';
    temp_str[5] = ' ';
    
    if (int_part >= 10)
    {
        temp_str[6] = '0' + (int_part / 10);
        temp_str[7] = '0' + (int_part % 10);
    }
    else
    {
        temp_str[6] = ' ';
        temp_str[7] = '0' + int_part;
    }
    
    temp_str[8] = '.';
    temp_str[9] = '0' + frac_part;
    temp_str[10] = 'C';
    temp_str[11] = ' ';
    
    if (use_sim_temp)
    {
        temp_str[12] = 'S';
        temp_str[13] = 'I';
        temp_str[14] = 'M';
    }
    else
    {
        temp_str[12] = 'R';
        temp_str[13] = 'E';
        temp_str[14] = 'L';
    }
    
    if (alarm_state == 1)
    {
        temp_str[15] = '!';
        LCD_WriteString(0, temp_str);
        LCD_WriteString(1, "!! OVER HEAT !!");
        return;
    }
    else if (alarm_state == 2)
    {
        temp_str[15] = '!';
        LCD_WriteString(0, temp_str);
        LCD_WriteString(1, "!! TOO COLD !!");
        return;
    }
    else
    {
        temp_str[15] = ' ';
    }
    temp_str[16] = '\0';
    LCD_WriteString(0, temp_str);
    
    // ===== 第二行 =====
    if (modify_mode == 1)
    {
        if (adjust_target == 0)
        {
            h_h = high_temp / 100;
            h_t = (high_temp % 100) / 10;
            h_o = high_temp % 10;
            
            temp_str[0] = 'H';
            temp_str[1] = ':';
            temp_str[2] = ' ';
            temp_str[3] = (h_h == 0) ? ' ' : ('0' + h_h);
            temp_str[4] = '0' + h_t;
            temp_str[5] = '.';
            temp_str[6] = '0' + h_o;
            temp_str[7] = 'C';
            temp_str[8] = ' ';
            temp_str[9] = '<';
            temp_str[10] = '-';
            temp_str[11] = '-';
            temp_str[12] = ' ';
            temp_str[13] = 'S';
            temp_str[14] = 'E';
            temp_str[15] = 'T';
            temp_str[16] = '\0';
        }
        else
        {
            l_h = low_temp / 100;
            l_t = (low_temp % 100) / 10;
            l_o = low_temp % 10;
            
            temp_str[0] = 'L';
            temp_str[1] = ':';
            temp_str[2] = ' ';
            temp_str[3] = (l_h == 0) ? ' ' : ('0' + l_h);
            temp_str[4] = '0' + l_t;
            temp_str[5] = '.';
            temp_str[6] = '0' + l_o;
            temp_str[7] = 'C';
            temp_str[8] = ' ';
            temp_str[9] = '<';
            temp_str[10] = '-';
            temp_str[11] = '-';
            temp_str[12] = ' ';
            temp_str[13] = 'S';
            temp_str[14] = 'E';
            temp_str[15] = 'T';
            temp_str[16] = '\0';
        }
        LCD_WriteString(1, temp_str);
    }
    else
    {
        h_h = high_temp / 100;
        h_t = (high_temp % 100) / 10;
        h_o = high_temp % 10;
        
        l_h = low_temp / 100;
        l_t = (low_temp % 100) / 10;
        l_o = low_temp % 10;
        
        temp_str[0] = 'H';
        temp_str[1] = ':';
        temp_str[2] = ' ';
        temp_str[3] = (h_h == 0) ? ' ' : ('0' + h_h);
        temp_str[4] = '0' + h_t;
        temp_str[5] = '.';
        temp_str[6] = '0' + h_o;
        temp_str[7] = 'C';
        temp_str[8] = ' ';
        temp_str[9] = 'L';
        temp_str[10] = ':';
        temp_str[11] = ' ';
        temp_str[12] = (l_h == 0) ? ' ' : ('0' + l_h);
        temp_str[13] = '0' + l_t;
        temp_str[14] = '.';
        temp_str[15] = '0' + l_o;
        temp_str[16] = 'C';
        temp_str[17] = '\0';
        
        LCD_WriteString(1, temp_str);
    }
}

// ==================== 检查蜂鸣器状态 ====================
void CheckBuzzerAlarm(void)
{
    if (buzzer_alarm_active && !BUZZER_IsAlarmActive())
    {
        buzzer_alarm_active = 0;
    }
}

// ==================== 主函数 ====================
void main(void)
{
    unsigned char key_num;
    unsigned char lcd_update_count = 0;
    P1 = 1;  // 设置 P1 为高电平，确保风扇初始状态为关闭
    
    // ===== 1. 初始化 =====
    KEY_Init();
    Timer0_Init();
    BUZZER_Init();
    LCD_Init();
    
    Delay_Ms(10);
    LCD_Clear();
    
    LoadThresholdFromEEPROM();
    
    // ===== 2. 检测传感器 =====
    CheckSensor();
    
    // ===== 3. 显示启动信息 =====
    ShowStartupInfo();
    
    // ===== 4. 设置初始温度 =====
    if (!sensor_ok)
    {
        use_sim_temp = 1;
        real_temp = 200;
        sim_temp = 200;
    }
    else
    {
        // 尝试读取一次真实温度
        real_temp = 200;
        // 立即读取一次
        ProcessTemperatureRead();
    }
    
    UpdateLCD();
    
    // ===== 5. 主循环 =====
    while (1)
    {
        BUZZER_PWM_Output();
        
        key_num = KEY_GetPress();
        
        // ===== SET 键 (P3.2) =====
        if (key_num == 3)
        {
            BUZZER_ShortBeep();
            
            if (modify_mode == 1)
            {
                modify_mode = 0;
                SaveThresholdToEEPROM();
                LCD_WriteString(0, "Saved!         ");
                LCD_WriteString(1, "                ");
                Delay_Ms(500);
            }
            
            UpdateLCD();
        }
        
        // ===== VIEW 键 (P3.3) =====
        if (key_num == 4)
        {
            BUZZER_ShortBeep();
            
            if (modify_mode == 0)
            {
                modify_mode = 1;
                adjust_target = 0;
            }
            else
            {
                adjust_target = !adjust_target;
            }
            UpdateLCD();
        }
        
        // ===== INC 键 (P3.1) =====
        if (modify_mode == 1 && key_num == 1)
        {
            BUZZER_ShortBeep();
            AdjustTemperature(0);
            UpdateLCD();
        }
        
        // ===== DEC 键 (P3.0) =====
        if (modify_mode == 1 && key_num == 2)
        {
            BUZZER_ShortBeep();
            AdjustTemperature(1);
            UpdateLCD();
        }
        
        // ===== 温度更新（每 500ms） =====
        read_counter++;
        if (read_counter >= 50)
        {
            read_counter = 0;
            
            ProcessTemperatureRead();
            MonitorTemperature();
            //FAN_Control(real_temp >= high_temp ? 1 : 0);
            
            lcd_update_count++;
            if (lcd_update_count >= 4)
            {
                lcd_update_count = 0;
                if (modify_mode == 0)
                {
                    UpdateLCD();
                }
            }
        }
        
        CheckBuzzerAlarm();
        Delay_Ms(5);
    }
}