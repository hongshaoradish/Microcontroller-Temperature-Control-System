#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <reg52.h>

// ==================== 引脚定义 ====================
sbit I2C_SCL = P2^1;
sbit I2C_SDA = P2^0;

// ==================== 器件地址 ====================
#define EEPROM_ADDR_WRITE  0xA0
#define EEPROM_ADDR_READ   0xA1

// ==================== 温度阈值地址（×10格式，0-999） ====================
#define ADDR_TEMP_LOWER_H   0x00   // 下限高字节
#define ADDR_TEMP_LOWER_L   0x01   // 下限低字节
#define ADDR_TEMP_UPPER_H   0x02   // 上限高字节
#define ADDR_TEMP_UPPER_L   0x03   // 上限低字节

// ==================== 函数声明 ====================

// I2C底层
void I2C_Start(void);
void I2C_Stop(void);
unsigned char I2C_SendByte(unsigned char dat);
unsigned char I2C_RecvByte(unsigned char ack);

// EEPROM读写
unsigned char EEPROM_WriteByte(unsigned int addr, unsigned char dat);
unsigned char EEPROM_ReadByte(unsigned int addr);

// 温度阈值读写（×10格式）
unsigned char EEPROM_SaveThreshold(unsigned int lower, unsigned int upper);
unsigned char EEPROM_ReadThreshold(unsigned int *lower, unsigned int *upper);

#endif