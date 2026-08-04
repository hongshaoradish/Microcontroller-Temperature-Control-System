#include "../inc/key.h"
#include "../inc/delay.h"

// ==================== 按键状态记录 ====================
static unsigned char key_inc_state = 0;
static unsigned char key_dec_state = 0;
static unsigned char key_set_state = 0;
static unsigned char key_view_state = 0;

// ==================== 按键按下标志 ====================
static unsigned char key_inc_pressed = 0;
static unsigned char key_dec_pressed = 0;
static unsigned char key_set_pressed = 0;
static unsigned char key_view_pressed = 0;

// ==================== 消抖计数器 ====================
static unsigned char key_inc_cnt = 0;   // 改为 unsigned char（最大255，够用）
static unsigned char key_dec_cnt = 0;
static unsigned char key_set_cnt = 0;
static unsigned char key_view_cnt = 0;

// ==================== 消抖时间（增加至 12）====================
#define DEBOUNCE_TIME  12  // 12 * 2ms = 24ms（原为 5，增加消抖时间）

void KEY_Init(void)
{
    KEY_INC = 1;
    KEY_DEC = 1;
    KEY_SET = 1;
    KEY_VIEW = 1;
    
    key_inc_state = 0;
    key_dec_state = 0;
    key_set_state = 0;
    key_view_state = 0;
    
    key_inc_pressed = 0;
    key_dec_pressed = 0;
    key_set_pressed = 0;
    key_view_pressed = 0;
    
    key_inc_cnt = 0;
    key_dec_cnt = 0;
    key_set_cnt = 0;
    key_view_cnt = 0;
}

// ==================== 非阻塞按键扫描（在定时器中断中调用）====================
void KEY_Scan(void)
{
    // ----- KEY_INC (P3.1) -----
    if (KEY_INC == 0)
    {
        if (key_inc_cnt < DEBOUNCE_TIME)
            key_inc_cnt++;
        
        // 消抖完成 且 状态为0（未触发过）
        if (key_inc_cnt >= DEBOUNCE_TIME && key_inc_state == 0)
        {
            key_inc_state = 1;       // 标记已触发
            key_inc_pressed = 1;     // 标记按下
        }
    }
    else
    {
        key_inc_cnt = 0;
        key_inc_state = 0;           // 释放后重置状态，允许下次触发
    }
    
    // ----- KEY_DEC (P3.0) -----
    if (KEY_DEC == 0)
    {
        if (key_dec_cnt < DEBOUNCE_TIME)
            key_dec_cnt++;
        
        if (key_dec_cnt >= DEBOUNCE_TIME && key_dec_state == 0)
        {
            key_dec_state = 1;
            key_dec_pressed = 1;
        }
    }
    else
    {
        key_dec_cnt = 0;
        key_dec_state = 0;
    }
    
    // ----- KEY_SET (P3.2) -----
    if (KEY_SET == 0)
    {
        if (key_set_cnt < DEBOUNCE_TIME)
            key_set_cnt++;
        
        if (key_set_cnt >= DEBOUNCE_TIME && key_set_state == 0)
        {
            key_set_state = 1;
            key_set_pressed = 1;
        }
    }
    else
    {
        key_set_cnt = 0;
        key_set_state = 0;
    }
    
    // ----- KEY_VIEW (P3.3) -----
    if (KEY_VIEW == 0)
    {
        if (key_view_cnt < DEBOUNCE_TIME)
            key_view_cnt++;
        
        if (key_view_cnt >= DEBOUNCE_TIME && key_view_state == 0)
        {
            key_view_state = 1;
            key_view_pressed = 1;
        }
    }
    else
    {
        key_view_cnt = 0;
        key_view_state = 0;
    }
}

// ==================== 获取按键状态（主循环调用）====================
unsigned char KEY_GetPress(void)
{
    unsigned char result = 0;
    
    if (key_inc_pressed)
    {
        key_inc_pressed = 0;  // 清除标志
        result = 1;
    }
    else if (key_dec_pressed)
    {
        key_dec_pressed = 0;
        result = 2;
    }
    else if (key_set_pressed)
    {
        key_set_pressed = 0;
        result = 3;
    }
    else if (key_view_pressed)
    {
        key_view_pressed = 0;
        result = 4;
    }
    
    return result;
}