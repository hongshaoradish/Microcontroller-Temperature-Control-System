#include "../inc/eeprom.h"
#include "../inc/delay.h"

// ==================== I2C 底层函数 ====================

void I2C_Start(void)
{
    I2C_SDA = 1;
    I2C_SCL = 1;
    Delay_Us(5);
    I2C_SDA = 0;
    Delay_Us(5);
    I2C_SCL = 0;
}

void I2C_Stop(void)
{
    I2C_SDA = 0;
    I2C_SCL = 1;
    Delay_Us(5);
    I2C_SDA = 1;
    Delay_Us(5);
}

unsigned char I2C_SendByte(unsigned char dat)
{
    unsigned char i;
    unsigned char ack;
    
    for (i = 0; i < 8; i++)
    {
        I2C_SCL = 0;
        Delay_Us(2);
        
        if (dat & 0x80)
            I2C_SDA = 1;
        else
            I2C_SDA = 0;
        
        dat <<= 1;
        Delay_Us(2);
        I2C_SCL = 1;
        Delay_Us(5);
    }
    
    I2C_SCL = 0;
    Delay_Us(2);
    I2C_SDA = 1;
    Delay_Us(2);
    I2C_SCL = 1;
    Delay_Us(5);
    
    ack = (I2C_SDA == 0) ? 1 : 0;
    I2C_SCL = 0;
    
    return ack;
}

unsigned char I2C_RecvByte(unsigned char ack)
{
    unsigned char i;
    unsigned char dat = 0;
    
    I2C_SDA = 1;
    
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        I2C_SCL = 0;
        Delay_Us(2);
        I2C_SCL = 1;
        Delay_Us(5);
        if (I2C_SDA)
            dat |= 0x01;
    }
    
    I2C_SCL = 0;
    Delay_Us(2);
    
    if (ack)
        I2C_SDA = 0;
    else
        I2C_SDA = 1;
    
    Delay_Us(2);
    I2C_SCL = 1;
    Delay_Us(5);
    I2C_SCL = 0;
    
    return dat;
}

// ==================== EEPROM 读写操作 ====================

unsigned char EEPROM_WriteByte(unsigned int addr, unsigned char dat)
{
    unsigned char ack;
    
    EA = 0;
    
    I2C_Start();
    ack = I2C_SendByte(EEPROM_ADDR_WRITE);
    if (!ack) { I2C_Stop(); EA = 1; return 0; }
    
    ack = I2C_SendByte((unsigned char)(addr & 0xFF));
    if (!ack) { I2C_Stop(); EA = 1; return 0; }
    
    ack = I2C_SendByte(dat);
    if (!ack) { I2C_Stop(); EA = 1; return 0; }
    
    I2C_Stop();
    EA = 1;
    Delay_Ms(10);  // 等待写入完成
    
    return 1;
}

unsigned char EEPROM_ReadByte(unsigned int addr)
{
    unsigned char dat = 0xFF;
    unsigned char ack;
    
    EA = 0;
    
    I2C_Start();
    ack = I2C_SendByte(EEPROM_ADDR_WRITE);
    if (!ack) { I2C_Stop(); EA = 1; return 0xFF; }
    
    ack = I2C_SendByte((unsigned char)(addr & 0xFF));
    if (!ack) { I2C_Stop(); EA = 1; return 0xFF; }
    
    I2C_Start();
    ack = I2C_SendByte(EEPROM_ADDR_READ);
    if (!ack) { I2C_Stop(); EA = 1; return 0xFF; }
    
    dat = I2C_RecvByte(0);
    I2C_Stop();
    
    EA = 1;
    
    return dat;
}

// ==================== 温度阈值读写（×10格式，0-999） ====================

unsigned char EEPROM_SaveThreshold(unsigned int lower, unsigned int upper)
{
    unsigned char result = 1;
    
    // 限制范围 0-999
    if (lower > 999) lower = 999;
    if (upper > 999) upper = 999;
    if (lower > upper) lower = upper;
    
    // 写入上限（高字节 + 低字节）
    result &= EEPROM_WriteByte(ADDR_TEMP_UPPER_H, (upper >> 8) & 0xFF);
    result &= EEPROM_WriteByte(ADDR_TEMP_UPPER_L, upper & 0xFF);
    
    // 写入下限（高字节 + 低字节）
    result &= EEPROM_WriteByte(ADDR_TEMP_LOWER_H, (lower >> 8) & 0xFF);
    result &= EEPROM_WriteByte(ADDR_TEMP_LOWER_L, lower & 0xFF);
    
    return result;
}

unsigned char EEPROM_ReadThreshold(unsigned int *lower, unsigned int *upper)
{
    unsigned int temp_lower, temp_upper;
    unsigned char h, l;
    
    // 读取上限
    h = EEPROM_ReadByte(ADDR_TEMP_UPPER_H);
    l = EEPROM_ReadByte(ADDR_TEMP_UPPER_L);
    temp_upper = ((unsigned int)h << 8) | l;
    
    // 读取下限
    h = EEPROM_ReadByte(ADDR_TEMP_LOWER_H);
    l = EEPROM_ReadByte(ADDR_TEMP_LOWER_L);
    temp_lower = ((unsigned int)h << 8) | l;
    
    // 检查是否未写入（全0xFF）
    if (temp_upper == 0xFFFF && temp_lower == 0xFFFF)
    {
        // 首次使用，返回默认值
        *lower = 150;   // 15.0℃
        *upper = 400;   // 40.0℃
        return 0;
    }
    
    // 检查数据是否有效（0-999）
    if (temp_upper > 999 || temp_lower > 999)
    {
        // 数据损坏，使用默认值
        *lower = 150;
        *upper = 400;
        return 0;
    }
    
    // 确保下限 ≤ 上限
    if (temp_lower > temp_upper)
    {
        *lower = temp_upper;
        *upper = temp_lower;
    }
    else
    {
        *lower = temp_lower;
        *upper = temp_upper;
    }
    
    return 1;
}