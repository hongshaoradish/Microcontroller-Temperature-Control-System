#ifndef __DS18B20_H__
#define __DS18B20_H__

#include <reg52.h>
#include <intrins.h>

// ==================== 引脚定义 ====================
sbit DS18B20_DQ = P3^7;

// ==================== DS18B20 命令 ====================
#define DS18B20_CMD_CONVERT    0x44
#define DS18B20_CMD_READ_SCR   0xBE
#define DS18B20_CMD_SKIP_ROM   0xCC

// ==================== 错误码定义 ====================
#define DS18B20_ERR_NONE       0x0000
#define DS18B20_ERR_RESET      0xFFFF
#define DS18B20_ERR_CRC        0xFFFE
#define DS18B20_ERR_ZERO       0xFFFD
#define DS18B20_ERR_RANGE      0xFFFC

// ==================== 函数声明 ====================
unsigned char DS18B20_Reset(void);
void DS18B20_WriteBit(unsigned char bit_value);
unsigned char DS18B20_ReadBit(void);
void DS18B20_WriteByte(unsigned char dat);
unsigned char DS18B20_ReadByte(void);
unsigned char DS18B20_Init(void);
void DS18B20_StartConvert(void);
unsigned int DS18B20_ReadTemp(void);
unsigned char DS18B20_ReadScratchpad(unsigned char *buffer);
unsigned char DS18B20_VerifyScratchpad(unsigned char *buffer);
unsigned char DS18B20_CRC8(unsigned char crc, unsigned char byte_data);

#endif