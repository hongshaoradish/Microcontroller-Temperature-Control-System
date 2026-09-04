此项目仅用于了解C51单片机以及练习Git和GitHub相关操作命令，仅实现最基本功能

C51单片机温度监控系统

目录
C51单片机温度监控系统	1
一、EEPROM存储器（I2C通信协议）	2
1、I2C总线	2
（1）特点	2
（2）信号	2
（3）写流程	3
（4）读流程	3
（5）函数	3
（6）函数封装	6
2、具体实现	8
（1）应用层（主函数调用）	8
（2）应用封装层（eeprom.c）	10
（3）基础读写层（eeprom.c）	14
（4）硬件层	17
二、温度传感器（1-wire通信协议）	17
1、1-wire协议	18
2、信号	18
（1）复位（初始化）	18
（2）写1（主机）	18
（3）写0（主机）	18
（4）读1	18
（5）读0	18
3、读取流程	18
（1）初始化（复位）	18
（2）写指令，跳过ROM（0xCC）	18
（3）写指令，启动转换（0x44）	18
（4）等待传感器读取温度	19
（5）再次初始化	19
（6）跳过ROM	19
（7）写指令，读取寄存器（0xBE）	19
（8）读取温度	19
（9）还原温度值	19
4、函数	19
（1）延时函数	19
（2）复位函数	19
（3）写入一个字节	19
（4）读取一个字节	20
5、函数封装	20
（1）读取传感器温度（返回原始数据，需要手动乘0,0625）	20
6、具体实现	21
三、LCD液晶显示模块	21
四、蜂鸣器	21
1、蜂鸣器的分类	21
（1）有源蜂鸣器	21
（2）无源蜂鸣器	21
2、三极管	22
（1）蜂鸣器必须接入三极管	22
（2）接口	22
（3）三极管类型	22
3、函数（有源蜂鸣器）	22
4、无源蜂鸣器（重点）	23
（1）产生的声音频率	23
（2）产生声音的大小	23
（3）参数计算	23
（4）简单延时法	24
（5）定时器中断法	24
五、按键输入状态	25
六、主函数逻辑	25
七、中断设计	25




一、EEPROM存储器（I2C通信协议）
1、I2C总线
（1）特点
1.有两根数据线SDA（串行数据线）和SCL（串行时钟线）
2.总线空闲时两根线都是高电平，当一个设备占用时会变成低电平，此时其他设备不可使用

（2）信号
1.起始信号S:
SDA：高电平跳变为低电平时（1 -> 0）
SCL：需要保持高电平

2.停止信号S:
SDA：低电平跳变为高电平（0 -> 1）
SCL：需要保持高电平

3.数据传输：
SCL：当时钟信号为高电平时，才能传输信号
SDA：在时钟信号为高电平时，SDA发送的高低电平才能被传输

注意：起始信号发送后，数据每传输8位，就要停止，等待对方回应，然后继续传输
流程：起始信号 → 第1个字节(8位数据) → 等待应答位(1位) → 第2个字节(8位数据) → 等待应答位(1位) → ... → 停止信号

4.应答信号A：
当发送方发送了8个数据位后，发送方要释放SDA（拉高电平，表示空闲），接收方拉低SDA（拉低电平表示占用），而接收方拉低SDA就表示确认信号

5.非应答信号：
与应答信号相反，当发送方释放SDA后，接收方没有进行任何动作，表示接受失败

（3）写流程
1.主机信号发送起始信号

2.主机发送8个位，其中前7位表示设备地址（设备地址一般是固定的，出厂就被设置），最后
一位表示 写（0） 标志

3.接收方发出应答

4.主机发送 要写入接收方对应寄存的地址（8位），等待接收方应答

5.主机发送要写入的数据（8位），等待接收方应答

6.主机发送停止信号

（4）读流程
1.主机发送起始信号

2.主机发送8个位，发送方地址（7位） + 写（0）标志，等待接收方应答

3.主机发送8位， 要读取的发送方对应寄存的地址，等待接收方应答

4.主机再次发送起始信号

5.主机发送8位，发送方地址（7位） + 读（1）标志，等待对方应答

6.接收方发送8位，发送方寄存对应位置的数据

7.主机发送非应答信号，或者停止信号

（5）函数
假设 SDA 在 P1.0    SCL在P1.1

1.延时函数
用于发送后等待数据发送完成

void I2C_Delay(void) {
    _nop_(); _nop_(); _nop_(); _nop_();
}

注意：一个_nop_()约等于一个机器周期， 一个_nop_()相当于1us


2.起始函数
// 1. 起始信号
void I2C_Start(void) {
    SDA = 1;
    SCL = 1;
    I2C_Delay();
    SDA = 0;      // SCL高电平时，SDA产生下降沿
    I2C_Delay();
    SCL = 0;      // 钳住总线，准备发送数据
}

先将SDA,SCL拉高，保证后续能够产生下降沿
然后  在SCL为高的时候 将SDA由高拉低，产生起始信号

3.停止函数
// 2. 停止信号
void I2C_Stop(void) {
    SDA = 0;
    SCL = 1;
    I2C_Delay();
    SDA = 1;      // SCL高电平时，SDA产生上升沿
    I2C_Delay();
}

先将SDA拉低，保证后续能够产生上升沿，同时保证SCL在高电平
然后 在SCL高电平的时候 将SDA由低拉高，产生停止信号

4.发送数据函数
// 3. 发送一个字节
bit I2C_SendByte(unsigned char dat) {
    unsigned char i;
    bit ack;

    for (i = 0; i < 8; i++) { // 循环8次发送一个字节
        SDA = (dat & 0x80) ? 1 : 0; // 从最高位开始发送
        dat <<= 1;
        SCL = 1;          // 拉高SCL，从机在此上升沿采样数据
        I2C_Delay();
        SCL = 0;          // 拉低SCL，准备发送下一位
        I2C_Delay();
    }

    // 释放SDA总线，接收从机的应答
    SDA = 1;
    SCL = 1;
    I2C_Delay();
    ack = SDA;            // 读取应答位，0为应答(ACK)，1为非应答(NACK)
    SCL = 0;
    I2C_Delay();
    return ack;
}
注意：这里的ack是发送方主动行为，并不是函数返回值
发送数据时从高位开始
发送数据时，先设定SDA，后拉高SCL表示发送，等待延时后，再拉低SCL，以此循环
8位数据发送完成后，拉高SDA，SCL，释放总线，等待发送方应答，
最后SCL拉低，获取SCL控制

5.读取数据函数
// 4. 接收一个字节
unsigned char I2C_ReadByte(bit ack) {
    unsigned char i, dat = 0;

    SDA = 1;              // 释放SDA，准备读取数据
    for (i = 0; i < 8; i++) {
        dat <<= 1;
        SCL = 1;
        I2C_Delay();
        if (SDA) dat |= 0x01; // 读取SDA上的数据位
        SCL = 0;
        I2C_Delay();
    }

    // 发送应答位
    SDA = ack ? 1 : 0;    // ack=0发送ACK，ack=1发送NACK
    SCL = 1;
    I2C_Delay();
    SCL = 0;
    SDA = 1;              // 释放SDA
    I2C_Delay();
    return dat;
}

先拉高SDA，释放SDA控制权
然后拉高SCL，用于后续读取SDA（这里和发送相反，发送是先准备数据，然后拉高SCL发送，读取是先拉高SCL，然后再读取）

读取完毕后，根据ack确定是否还继续读
（注意ack是手动写的函数参数，在传参时，0表示继续读，1表示停止）
data1 = I2C_ReadByte(0); // 读第1个字节，发送ACK（0），告诉从机“继续发”
data2 = I2C_ReadByte(1); // 读第2个字节，发送NACK（1），告诉从机“结束”
I2C_Stop();

（6）函数封装
1.写一个字节到EEPROM

函数名：AT24C02_WriteByte
功能：向AT24C02指定地址写入一个字节
参数：addr - 要写入的内存地址 (0x00~0xFF)，dat  - 要写入的数据
返回值：bit - 返回0表示写入成功，返回1表示写入失败（从机无应答）


bit AT24C02_WriteByte(unsigned char addr, unsigned char dat) {
    bit ack;

    I2C_Start();                      // 1. 发送起始信号

    ack = I2C_SendByte(0xA0);         // 2. 发送设备地址 + 写标志 (0xA0)
    if (ack) {                        // 如果从机无应答，直接停止并返回错误
        I2C_Stop();
        return 1;
    }

    ack = I2C_SendByte(addr);         // 3. 发送内存地址
    if (ack) {
        I2C_Stop();
        return 1;
    }

    ack = I2C_SendByte(dat);          // 4. 发送要写入的数据
    if (ack) {
        I2C_Stop();
        return 1;
    }

    I2C_Stop();                       // 5. 发送停止信号

    // 注意：EEPROM在接收到停止信号后，会进入内部写周期（约5~10ms）
    // 在此期间，EEPROM不会响应任何I2C通信，需要等待或使用查询方式

    return 0;                         // 写入成功
}

2.从EEPROM读取一个字节
函数名：AT24C02_ReadByte
功能：从AT24C02指定地址读取一个字节
参数：addr - 要读取的内存地址 (0x00~0xFF)
返回值：unsigned char - 读取到的数据

unsigned char AT24C02_ReadByte(unsigned char addr) {
    unsigned char dat;
    bit ack;

    // -------- 阶段一：伪写（告诉从机要读哪个地址） --------
    I2C_Start();                      // 1. 发送起始信号

    ack = I2C_SendByte(0xA0);         // 2. 发送设备地址 + 写标志 (0xA0)
    if (ack) {                        // 如果从机无应答，直接停止并返回错误值
        I2C_Stop();
        return 0xFF;
    }

    ack = I2C_SendByte(addr);         // 3. 发送要读取的内存地址
    if (ack) {
        I2C_Stop();
        return 0xFF;
    }

    // -------- 阶段二：重启并切换为读方向 --------
    I2C_Start();                     // 4. 发送重启起始信号 (没有停止!)

    ack = I2C_SendByte(0xA1);        // 5. 发送设备地址 + 读标志 (0xA1)
    if (ack) {
        I2C_Stop();
        return 0xFF;
    }

    // -------- 阶段三：读取数据 --------
    dat = I2C_ReadByte(1);           // 6. 读取1个字节，并发送NACK (参数1)
    // 因为只读一个字节，所以读完直接给NACK，告诉从机不要再发了

    I2C_Stop();                      // 7. 发送停止信号

    return dat;
}

2、具体实现

（1）应用层（主函数调用）

1.加载阈值函数
LoadThresholdFromEEPROM();

作用：在上电以后，读取上一次关机时保存的上下限，加载到内存中使用

相关函数：
EEPROM_ReadThreshold(&lower, &upper)驱动层函数，读取EEPROM当中的数字


位置：主项目中 main.c    不在主函数中


void LoadThresholdFromEEPROM(void)
{
    unsigned int lower, upper;
    
if (EEPROM_ReadThreshold(&lower, &upper))              
//调用EEPROM_ReadThreshold()函数（驱动层），读取上一次写入的数值，读取失败时返回0
{
//注意  low_temp、high_temp是全局变量，不需要返回，即可在主函数的其他区域使用
        low_temp = lower;         
        high_temp = upper;
    }
else
//读取失败后，采用默认上下限，并且重新写入默认值到EEPROM中
    {
        low_temp = 150;
        high_temp = 380;
        EEPROM_SaveThreshold(low_temp, high_temp);
//重新写入默认值到EEPROM中
    }
    //上下边界保护，防止数据溢出，防止上下限交换
    if (high_temp > 999) high_temp = 999;
    if (low_temp > 999) low_temp = 999;
    if (low_temp > high_temp) low_temp = high_temp;
}




主函数的调用位置：
初始化部分，不在主循环当中
    // ===== 1. 初始化 =====
    KEY_Init();
    Timer0_Init();
    BUZZER_Init();
    LCD_Init();
    
    Delay_Ms(10);
    LCD_Clear();
    
    LoadThresholdFromEEPROM();       <-------调用函数
    
    // ===== 2. 检测传感器 =====



2.检查阈值合法函数
调用函数：SaveThresholdToEEPROM()
函数声明：void SaveThresholdToEEPROM(void)

作用：在保存当前阈值时，检查阈值是否合法，避免上下限交换，或者数值溢出情况，正确数值确定后在本函数内调用  保存阈值函数EEPROM_SaveThreshold(low_temp, high_temp)  进行上下限保存


相关函数声明：EEPROM_SaveThreshold(low_temp, high_temp) 保存阈值函数     应用封装层


函数定义：
void SaveThresholdToEEPROM(void)
{
    if (high_temp > 999) high_temp = 999;
    if (low_temp > 999) low_temp = 999;
    if (low_temp > high_temp) low_temp = high_temp;
    
    EEPROM_SaveThreshold(low_temp, high_temp);
}


调用函数位置：
主循环当中            
if (modify_mode == 1)               //如果在修改模式下（modify_mode == 1）按下set键
 {
   modify_mode = 0;                 //修改状态回到正常模式
   SaveThresholdToEEPROM();        //验证合法性，并保存当前上下限
   LCD_WriteString(0, "Saved!         ");
   LCD_WriteString(1, "                ");
   Delay_Ms(500);
 }

在修改模式中修改数值后，按下保存按键（set）时触发，set按键只参与保存，在正常情况下按下set键没有操作
只有当按下view键  进入修改模式后，此时按下set键可以保存修改值，并回到正常模式

说明：modify_mode == 1    修改标志位（1：修改模式   0：正常模式）




（2）应用封装层（eeprom.c）

1.上下限读取函数
调用函数： EEPROM_ReadThreshold(&lower, &upper)
函数声明：unsigned char EEPROM_ReadThreshold(unsigned int *lower, unsigned int *upper)

作用：从EEPROM当中正确读取上下限，并且正确还原上下限温度，校验数据合法性
EEPROM的数据是由两个字节共同存储一个数据，且EEPROM未写入情况下为1，不是0

本质：温度范围0 ~ 999 x 10格式  超出了一个字节的范围，需要两个字节共同存储


相关函数声明：基础读写层
unsigned char EEPROM_ReadByte(unsigned int addr)
unsigned char EEPROM_WriteByte(unsigned int addr, unsigned char dat)
作用：按字节读取或者写入EEPROM数据


函数定义：
unsigned char EEPROM_ReadThreshold(unsigned int *lower, unsigned int *upper)
{
    unsigned int temp_lower, temp_upper;              
    unsigned char h, l;

//说明 ADDR_TEMP_UPPER_H等是宏定义，在eeprom.h当中定义，是一个内存地址    

    // 读取上限
    h = EEPROM_ReadByte(ADDR_TEMP_UPPER_H);         //读取上限高字节
    l = EEPROM_ReadByte(ADDR_TEMP_UPPER_L);         //读取上限低字节
    temp_upper = ((unsigned int)h << 8) | l;        //拼接数据
    
    // 读取下限
    h = EEPROM_ReadByte(ADDR_TEMP_LOWER_H);
    l = EEPROM_ReadByte(ADDR_TEMP_LOWER_L);
    temp_lower = ((unsigned int)h << 8) | l;
    
// 检查是否未写入（全0xFF）
//EEPROM数据在未写入时是1，而不是0，当上下限全为1时，逻辑与为1，进入
    if (temp_upper == 0xFFFF && temp_lower == 0xFFFF)
    {
        // 首次使用，返回默认值
        *lower = 150;   // 15.0℃
        *upper = 400;   // 40.0℃
        return 0;
    }
    
    // 检查数据是否有效（0-999）
    if (temp_upper > 999 || temp_lower > 999)
    {
        // 数据损坏，使用默认值
        *lower = 150;                 //指针解引用
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
    
    return 1;            //返回1表示读取成功，0表示首次使用或者数据损坏
}



函数调用位置：
main.c  中LoadThresholdFromEEPROM()函数内被调用
此函数LoadThresholdFromEEPROM(void)，在主函数初始化部分被调用，用于初次加载阈值

void LoadThresholdFromEEPROM(void)
{
    unsigned int lower, upper;
    
    if (EEPROM_ReadThreshold(&lower, &upper))   //函数被调用
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





2.保存上下限进入EEPROM函数
调用函数：EEPROM_SaveThreshold(low_temp, high_temp)
函数声明：unsigned char EEPROM_SaveThreshold(unsigned int lower, unsigned int upper)

作用：验证即将存入数据的合法性，将数据拆分为两个字节分别存入EEPROM


相关函数：基础读写层
unsigned char EEPROM_ReadByte(unsigned int addr)
unsigned char EEPROM_WriteByte(unsigned int addr, unsigned char dat)
作用：按字节读取或者写入EEPROM数据





函数定义：
unsigned char EEPROM_SaveThreshold(unsigned int lower, unsigned int upper)
{
    unsigned char result = 1;
    
    // 限制范围 0-999
    if (lower > 999) lower = 999;
    if (upper > 999) upper = 999;
    if (lower > upper) lower = upper;
    
    // 写入上限（高字节 + 低字节） 与运算 1位置保留，0位置置0
    result &= EEPROM_WriteByte(ADDR_TEMP_UPPER_H, (upper >> 8) & 0xFF);
    result &= EEPROM_WriteByte(ADDR_TEMP_UPPER_L, upper & 0xFF);
    
    // 写入下限（高字节 + 低字节）
    result &= EEPROM_WriteByte(ADDR_TEMP_LOWER_H, (lower >> 8) & 0xFF);   //按字节写入
    result &= EEPROM_WriteByte(ADDR_TEMP_LOWER_L, lower & 0xFF);
    
    //EEPROM_WriteByte函数如果写入成功会返回1，与result进行与运算
    return result;  //如果result = 1 & 1 & 1 = 1 表示成功,反之如果当中出现0，result = 0表示失败
}


函数调用位置：
main.c  中LoadThresholdFromEEPROM()函数内被调用
此函数LoadThresholdFromEEPROM(void)，在主函数初始化部分被调用，用于初次加载阈值时检测到EEPROM当中没有数据时，使用默认数值，并将默认数值使用EEPROM_SaveThreshold()函数存入EEPROM


位置1：
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



位置2：
在修改上下限后，使用EEPROM_SaveThreshold()存入EEPROM
void SaveThresholdToEEPROM(void)
{
    if (high_temp > 999) high_temp = 999;
    if (low_temp > 999) low_temp = 999;
    if (low_temp > high_temp) low_temp = high_temp;
    
    EEPROM_SaveThreshold(low_temp, high_temp);
}







（3）基础读写层（eeprom.c）

1.读取字节函数
函数调用：EEPROM_ReadByte(ADDR)
函数声明：unsigned char EEPROM_ReadByte(unsigned int addr)

作用：用于在EEPROM读取最小单位数据（1字节）


相关函数声明：硬件层
void I2C_Start(void); 起始函数
void I2C_Stop(void);停止函数
unsigned char I2C_SendByte(unsigned char dat);发送函数
unsigned char I2C_RecvByte(unsigned char ack);接收函数



读流程：
1.主机发送起始信号
2.主机发送8个位，发送方地址（7位） + 写（0）标志，等待接收方应答
3.主机发送8位， 要读取的发送方对应寄存的地址，等待接收方应答
4.主机再次发送起始信号
5.主机发送8位，发送方地址（7位） + 读（1）标志，等待对方应答
6.接收方发送8位，发送方寄存对应位置的数据
7.主机发送非应答信号，或者停止信号


函数定义：
unsigned char EEPROM_ReadByte(unsigned int addr)
{
    unsigned char dat = 0xFF;   //dat储存读取的数据，0xFF表示失败
    unsigned char ack;          //从机应答信号位
    
    EA = 0;                      //关闭中断，避免定时器中断影响，原因：I2C精度要求极高
    
//1.主机发送起始信号
I2C_Start();      

//2.定位EEPROM设备          
ack = I2C_SendByte(EEPROM_ADDR_WRITE); 
    if (!ack) { I2C_Stop(); EA = 1; return 0xFF; } //等待从机应答
    
//3.伪写入，告知EEPROM要读取的地址
    ack = I2C_SendByte((unsigned char)(addr & 0xFF));
    if (!ack) { I2C_Stop(); EA = 1; return 0xFF; }
    
//4.主机再次发送起始信号
I2C_Start();
//5.发送读取命令，并附上读取地址
    ack = I2C_SendByte(EEPROM_ADDR_READ);
    if (!ack) { I2C_Stop(); EA = 1; return 0xFF; }
    
    //6.读取数据
dat = I2C_RecvByte(0);
//7.主机发送停止信号
    I2C_Stop();
    
//恢复定时器功能
    EA = 1;
    
    return dat;//返回读取到的一个字节数据
}


2.写入字节函数
函数调用：EEPROM_ReadByte(ADDRH)
函数声明：unsigned char EEPROM_WriteByte(unsigned int addr, unsigned char dat)

作用：用于在EEPROM写入最小单位数据（1字节）


相关函数声明：硬件层
void I2C_Start(void); 起始函数
void I2C_Stop(void);停止函数
unsigned char I2C_SendByte(unsigned char dat);发送函数
unsigned char I2C_RecvByte(unsigned char ack);接收函数

写流程：
1.主机信号发送起始信号
2.主机发送8个位，其中前7位表示设备地址（设备地址一般是固定的，出厂就被设置），最后
一位表示 写（0） 标志
3.接收方发出应答
4.主机发送 要写入接收方对应寄存的地址（8位），等待接收方应答
5.主机发送要写入的数据（8位），等待接收方应答
6.主机发送停止信号



函数定义：
unsigned char EEPROM_WriteByte(unsigned int addr, unsigned char dat)
{
    unsigned char ack;    //从机应答信号位
    
    EA = 0;                //关闭中断，避免定时器中断影响，原因：I2C精度要求极高
    
    //1.主机信号发送起始信号
I2C_Start();
//2.发送要读取的设备地址，和写标志位
ack = I2C_SendByte(EEPROM_ADDR_WRITE);
//3.接收方发出应答
    if (!ack) { I2C_Stop(); EA = 1; return 0; }
    
    //4.主机发送 要写入接收方对应寄存的地址（8位），等待接收方应答
    ack = I2C_SendByte((unsigned char)(addr & 0xFF));
    if (!ack) { I2C_Stop(); EA = 1; return 0; }
    
//5.主机发送要写入的数据（8位），等待接收方应答
    ack = I2C_SendByte(dat);
    if (!ack) { I2C_Stop(); EA = 1; return 0; }
    
//6.主机发送停止信号
I2C_Stop();

//恢复定时器功能
    EA = 1;
    Delay_Ms(10);  // 等待写入完成
    
    return 1;
}




（4）硬件层
4个底层函数

I2C_Start()       起始函数
I2C_Stop()        停止函数
I2C_SendByte()    发送函数
I2C_RecvByte()    接收函数

调用位置
unsigned char EEPROM_WriteByte(unsigned int addr, unsigned char dat)
unsigned char EEPROM_ReadByte(unsigned int addr)





二、温度传感器（1-wire通信协议）
1、1-wire协议
（1）特点
1.只有一条数据线（DQ），没有时钟线
2.有初始化指令（复位），用于提醒设备准备接收指令
3.根据电压保持时间识别0与1
4.读取和写入都是低位在前（和I2C协议相反）

2、信号

（1）复位（初始化）
主机：拉低DQ线480us然后释放（置1）
从机：如果接收到信号，则会拉低DQ（置0），提醒主机传感器存在

（2）写1（主机）
拉低DQ（1~15us）
然后拉高DQ（整个过程需要保持60~120us）

（3）写0（主机）
拉低DQ
依旧保持拉低DQ（整个过程需要保持60~120us）

（4）读1
（主机）拉低DQ（1~15us）
然后释放，拉高DQ
（从机）会将DQ拉高或拉低以表示数据

（5）读0
（主机）拉低DQ（1~15us）
然后释放，拉高DQ
（从机）会将DQ拉高或拉低以表示数据

3、读取流程
（1）初始化（复位）
1.主机拉低DQ保持（480us），然后释放，等待接受信号
2.主机读取DQ电平，如果是0表示传感器存在，1表示不存在

（2）写指令，跳过ROM（0xCC）
意义：跳过ROM意义是，告诉总线上全部设备准备接收指令（如果有多个设备则会冲突，需要指定ROM）
1.跳过ROM需要传输指令：0xCC

（3）写指令，启动转换（0x44）
1.启动转换指：让传感器开始测量温度，并保存温度在寄存器中

（4）等待传感器读取温度

（5）再次初始化
意义：结束上一次通信，重新开始新的一轮通信

（6）跳过ROM

（7）写指令，读取寄存器（0xBE）

（8）读取温度
1.数据是两个字节
2.DS18B20温度有效值是低到高12位，剩余位数全0表示正数，全1表示负数

（9）还原温度值
temp = (tempH << 8) | tempL;合并两个字节
realtemp = temp * 0.0625   真实温度要用原始数据 乘 0.0625

4、函数
假设DQ在P1.0口上
（1）延时函数
// 微秒级延时，时序要求严格，需根据晶振调整
void delay_us(unsigned int t) {
    while (t--) {
        _nop_(); _nop_(); _nop_(); _nop_(); // 约 4us (12MHz晶振下)
    }
}

（2）复位函数
bit DS18B20_Reset() {
    bit presence = 1;
    DQ = 0;             // 主机拉低总线
    delay_us(500);      // 保持低电平至少 480us
    DQ = 1;             // 释放总线
    delay_us(60);       // 等待 15~60us
    presence = DQ;      // 读取 DS18B20 的存在脉冲 (0=存在, 1=不存在)
    delay_us(200);      // 等待剩余时隙结束
    return presence;    // 返回 0 表示初始化成功
}
（3）写入一个字节
写位之前都应该先拉低DQ
然后短暂延时
再从最低位发送（保持60us）
释放总线，短暂延时
数据右移，准备下一位
// 向 DS18B20 写入一个字节 (低位在前)
void DS18B20_WriteByte(unsigned char dat) {
    unsigned char i;
    for (i = 0; i < 8; i++) {
        DQ = 0;         // 拉低总线，产生写时隙起始
        _nop_();        // 短暂延时
        // 根据要发送的位，决定是写0还是写1
        DQ = dat & 0x01; // 最低位先发送
        delay_us(60);   // 保持电平至少60us
        DQ = 1;         // 释放总线
        _nop_();
        dat >>= 1;      // 准备发送下一位
    }
}

（4）读取一个字节
读取前先右移一位（右移不会丢弃最低位）
// 从 DS18B20 读取一个字节 (低位在前)
unsigned char DS18B20_ReadByte() {
    unsigned char i, dat = 0;
    for (i = 0; i < 8; i++) {
        dat >>= 1;        // 先右移一位，为接收新位做准备
        DQ = 0;           // 拉低总线，产生读时隙起始
        _nop_(); _nop_(); // 保持至少 1us
        DQ = 1;           // 释放总线，让 DS18B20 控制
        _nop_(); _nop_(); // 稍作延时，在 15us 内读取
        if (DQ) {
            dat |= 0x80;  // 如果读取到高电平，则最高位置1
        }
        delay_us(60);     // 等待整个读时隙完成 (至少60us)
    }
    return dat;
}

5、函数封装
（1）读取传感器温度（返回原始数据，需要手动乘0,0625）
unsigned int DS18B20_GetTemperature() {
    unsigned char tempL, tempH;
    unsigned int temp;

    // 1. 初始化，并检查DS18B20是否存在
    if (DS18B20_Reset()) {
        return 0; // 设备不存在
    }

    // 2. 发送命令：跳过ROM (0xCC) + 启动转换 (0x44)
    DS18B20_WriteByte(0xCC);
    DS18B20_WriteByte(0x44);

    // 3. 等待转换完成 (需要约750ms，这里简单用延时)
    delay_us(750000); // 实际项目建议用定时器或更精确的延时

    // 4. 再次初始化
    DS18B20_Reset();

    // 5. 发送命令：跳过ROM (0xCC) + 读暂存器 (0xBE)
    DS18B20_WriteByte(0xCC);
    DS18B20_WriteByte(0xBE);

    // 6. 读取温度值的低字节和高字节
    tempL = DS18B20_ReadByte();
    tempH = DS18B20_ReadByte();

    // 7. 合并结果
    temp = (tempH << 8) | tempL;
    return temp; // 返回原始12位数据，实际温度 = temp * 0.0625
}


6、具体实现
三、LCD液晶显示模块





四、蜂鸣器
1、蜂鸣器的分类
（1）有源蜂鸣器
1.自带振荡器
2.由于自带振荡器频率固定，无法发出多种声音
3.成本较高

（2）无源蜂鸣器
1.不带振荡器，需要使用单片机IO口输出特定频率来驱动
2.由于频率由单片机控制，可以发出的声音多
3.成本低

2、三极管
（1）蜂鸣器必须接入三极管
因为蜂鸣器需要电流大，IO口无法提供，必须要三极管来提供大电流

（2）接口
1.基极连接 IO口
2.集电极或者发射极连接到蜂鸣器上

（3）三极管类型
PNP型：低电平触发，输出0时蜂鸣器发声
NPN型：高电平触发，输出1时蜂鸣器发声




3、函数（有源蜂鸣器）
void buzzer_loop1(unsigned int on_time_ms, unsigned int off_time_ms, unsigned int total_times)
{
    unsigned int count = 0;

    if (total_times == 0)  // 无限循环
    {
        while (1)
        {
            Buzzer = 0;              // 低电平触发，蜂鸣器响
            delay_ms(on_time_ms);
            Buzzer = 1;              // 高电平，蜂鸣器停
            delay_ms(off_time_ms);
        }
    }
    else  // 有限次数循环
    {
        for (count = 0; count < total_times; count++)
        {
            Buzzer = 0;
            delay_ms(on_time_ms);
            Buzzer = 1;
            delay_ms(off_time_ms);
        }
    }
}



4、无源蜂鸣器（重点）

（1）产生的声音频率
1.频率 完全由IO口电平反转周期决定

2.公式:
公式关系： 方波频率f = 1/周期​。如果让引脚高电平持续 t 微秒，低电平持续 t微秒，那么输出频率f = 1/2t

3.代码中的关键参数：
定时器初值（TH0/TL0）： 该数值决定了定时器多久溢出一次（即中断间隔）。中断间隔 = 翻转间隔，间隔越短，频率越高，声音越尖锐。

延时函数数值： 如果使用delay()，括号内的循环变量值直接决定延时长短，进而决定频率。
人耳范围： 人类听觉在 20Hz ~ 20kHz，但蜂鸣器最敏感、最响亮的频段通常在 2kHz ~ 5kHz。低于1kHz声音发闷，高于8kHz声音刺耳且音量会明显下降。

（2）产生声音的大小
1.声音大小由施加在蜂鸣器功率决定，

2.高电平占比 和 低电平占比各占一半时，声音最大
占空比 10% ~ 30%（低电平时间短）： 振膜得不到足够能量推开，声音变小变弱。
占空比 90%（长时间高电平）： 蜂鸣器内部电流接近恒定，振膜在单侧停滞，反而不发声
（这就是为什么直接给高电平或低电平它不响的原因）

（3）参数计算
1.要发出 1kHz 的声音（中等偏低音）：
周期 = 1/1000 = 1ms。（即 1/1000秒）
需要每 0.5ms（500微秒）翻转一次。
机器周期 = 12/12MHz = 1微秒。
定时器计数值 = 500 / 1 = 500。
因为8位定时器最大只能计255，所以 500 > 255，此频率必须用16位定时器模式。
初值 = 65536 - 500 = 65036（即 0xFE0C）

2.要发出 4kHz 的声音（响亮清脆）：
周期 = 0.25ms（即1/4000秒）
翻转间隔 = 125微秒。
定时器计数值 = 125。
因为 125 < 255，用8位自动重装模式即可。
初值 = 256 - 125 = 131（即 0x83）




（4）简单延时法
优点：不需要中断，代码简单
缺点：使用时需要占用全部CPU，CPU在发声时不能做其他事情
#include <reg51.h>
sbit buzzer = P1 ^ 5; // 定义蜂鸣器引脚

// 延时1ms函数 (12MHz晶振，粗略延时)
void delay1ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 110; j++);
}

// 产生500Hz方波，持续time个周期
void beep_500Hz(unsigned int time) {
    unsigned int i;
    for (i = 0; i < time; i++) {
        buzzer = 0;
        delay1ms(1);   // 延时1ms
        buzzer = 1;
        delay1ms(1);   // 延时1ms
    }
}

（5）定时器中断法
优点：定时器在满值时自动反转IO口，CPU可以进行其他任务
缺点：程序复杂
#include <reg51.h>
sbit BEEP = P3 ^ 0; // 定义蜂鸣器引脚

// 定时器0初始化 (方式2，自动重装，250us中断一次)
void Timer0_Init() {
    TMOD = 0x02;    // 8位自动重装模式
    TH0 = 0x06;
    TL0 = 0x06;
    ET0 = 1;        // 开启定时器0中断
    EA = 1;         // 开启总中断
    // TR0 = 1;      // 可在需要时启动
}

// 定时器0中断服务程序
void Timer0_Isr() interrupt 1 {
    BEEP = ~BEEP;   // 自动翻转电平
}

五、按键输入状态



六、主函数逻辑


七、中断设计