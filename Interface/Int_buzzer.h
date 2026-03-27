#ifndef __INT_BUZZER_H__
#define __INT_BUZZER_H__

#include "tim.h"

#define BUZZER_FREQ_1 261
#define BUZZER_FREQ_2 293
#define BUZZER_FREQ_3 329
#define BUZZER_FREQ_4 349
#define BUZZER_FREQ_5 392
#define BUZZER_FREQ_6 440

/**
 * @brief 开启蜂鸣器
 *
 */
void Int_buzzer_on(void);

/**
 * @brief 关闭蜂鸣器
 *
 */
void Int_buzzer_off(void);

/**
 * @brief 设置蜂鸣器频率
 *
 */
void Int_buzzer_set(uint16_t freq);

/**
 * @brief 播放音乐
 *
 */
void Int_buzer_play(void);

#endif /* __INT_BUZZER_H__ */
