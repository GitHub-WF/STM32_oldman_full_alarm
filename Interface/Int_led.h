#ifndef __INT_LED_H__
#define __INT_LED_H__

#include "main.h"

#define LED_GREEN 0xff0000
#define LED_RED 0x00ff00
#define LED_BLUE 0x0000ff

/**
 * @brief 发送数据到LED，后24位有效
 *
 * @param data 表示不同的颜色
 */
void Int_led_set(uint32_t data);

/**
 * @brief led闪烁
 *
 * @param color 闪烁颜色
 * @param ms 闪烁间隔
 */
void Int_led_blink(uint32_t color, uint16_t ms);

/**
 * @brief 彩虹灯
 *
 * @param ms 切换时间
 */
void Int_led_rainbow(uint16_t ms);

#endif /* __INT_LED_H__ */
