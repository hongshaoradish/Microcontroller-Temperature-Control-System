#include "../inc/ds18b20.h"
#include "../inc/delay.h"

// ===== 复位 =====
unsigned char DS18B20_Reset(void)
{
    unsigned char presence;
    
    DS18B20_DQ = 0;
    Delay_480us();
    DS18B20_DQ = 1;
    Delay_15us();
    presence = DS18B20_DQ;
    Delay_550us();
    
    return (presence == 0);
}

// ===== 写位 =====
void DS18B20_WriteBit(unsigned char bit_value)
{
    DS18B20_DQ = 0;
    _nop_(); _nop_();
    
    if (bit_value) {
        Delay_2us();
        DS18B20_DQ = 1;
        Delay_60us();
    } else {
        Delay_60us();
        DS18B20_DQ = 1;
        Delay_2us();
    }
}

// ===== 读位 =====
unsigned char DS18B20_ReadBit(void)
{
    unsigned char bit_value;
    
    DS18B20_DQ = 0;
    Delay_2us();
    DS18B20_DQ = 1;
    Delay_5us();
    bit_value = DS18B20_DQ;
    Delay_60us();
    
    return bit_value;
}

// ===== 写字节 =====
void DS18B20_WriteByte(unsigned char dat)
{
    unsigned char i;
    
    for (i = 0; i < 8; i++) {
        DS18B20_WriteBit(dat & 0x01);
        dat >>= 1;
    }
}

// ===== 读字节 =====
unsigned char DS18B20_ReadByte(void)
{
    unsigned char i;
    unsigned char dat = 0;
    
    for (i = 0; i < 8; i++) {
        dat >>= 1;
        if (DS18B20_ReadBit())
            dat |= 0x80;
    }
    
    return dat;
}

// ===== 初始化 =====
unsigned char DS18B20_Init(void)
{
    unsigned char result;
    unsigned char retry = 0;
    
    DS18B20_DQ = 1;
    Delay_Ms(10);
    
    do {
        EA = 0;
        result = DS18B20_Reset();
        EA = 1;
        retry++;
        if (!result) {
            Delay_Ms(10);
        }
    } while (!result && retry < 3);
    
    return result;
}

// ===== CRC-8 =====
unsigned char DS18B20_CRC8(unsigned char crc, unsigned char byte_data)
{
    unsigned char i;
    
    crc ^= byte_data;
    for (i = 0; i < 8; i++) {
        if (crc & 0x01) {
            crc = (crc >> 1) ^ 0x8C;
        } else {
            crc >>= 1;
        }
    }
    return crc;
}

// ===== 校验 =====
unsigned char DS18B20_VerifyScratchpad(unsigned char *buffer)
{
    unsigned char i;
    unsigned char crc = 0;
    
    for (i = 0; i < 8; i++) {
        crc = DS18B20_CRC8(crc, buffer[i]);
    }
    
    return (crc == buffer[8]);
}

// ===== 读取温度（核心函数）=====
unsigned int DS18B20_ReadTemp(void)
{
    unsigned char buffer[9];
    unsigned int temp;
    unsigned int result;
    bit is_negative = 0;
    unsigned char i;
    
    EA = 0;
    
    // 1. 复位
    if (!DS18B20_Reset()) {
        EA = 1;
        return DS18B20_ERR_RESET;
    }
    
    // 2. 发送转换命令
    DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(DS18B20_CMD_CONVERT);
    
    // 3. 等待转换完成（750ms）
    EA = 1;  // 转换期间允许中断
    Delay_Ms(750);
    EA = 0;
    
    // 4. 再次复位
    if (!DS18B20_Reset()) {
        EA = 1;
        return DS18B20_ERR_RESET;
    }
    
    // 5. 发送读命令
    DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(DS18B20_CMD_READ_SCR);
    
    // 6. 读取9字节
    for (i = 0; i < 9; i++) {
        buffer[i] = DS18B20_ReadByte();
    }
    
    EA = 1;
    
    // 7. CRC校验
    if (!DS18B20_VerifyScratchpad(buffer)) {
        return DS18B20_ERR_CRC;
    }
    
    // 8. 解析温度
    temp = ((unsigned int)buffer[1] << 8) | buffer[0];
    
    if (temp == 0x0000) {
        return DS18B20_ERR_ZERO;
    }
    if (temp == 0xFFFF) {
        return DS18B20_ERR_RESET;
    }
    
    if (temp & 0x8000) {
        temp = ~temp + 1;
        is_negative = 1;
    }
    
    if (temp > 1250) {
        return DS18B20_ERR_RANGE;
    }
    
    result = (unsigned int)((unsigned long)temp * 625 / 1000);
    
    if (is_negative) {
        result |= 0x8000;
    }
    
    return result;
}