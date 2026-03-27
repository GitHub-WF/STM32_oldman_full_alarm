#ifndef __INT_STEP_H__
#define __INT_STEP_H__

#include "i2c.h"

#define ADDR 0x4E

#define CHIP_ID 0x01
#define USER_SET 0xC3
#define STEP_CNT_L 0xC4
#define STEP_CNT_M 0xC5
#define STEP_CNT_H 0xC6


/**
 * @brief 初始化DS3553芯片，设置user_set寄存器
 *
 */
void Int_step_init(void);

/**
 * @brief 获取计步器数值
 *
 * @return uint32_t
 */
uint32_t Int_step_get_count(void);

#endif /* __INT_STEP_H__ */
