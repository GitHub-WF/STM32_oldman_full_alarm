#ifndef __APP_WARNING_H__
#define __APP_WARNING_H__

#include "main.h"
#include "Int_buzzer.h"
#include "Int_led.h"
#include "Int_mpu6050.h"
#include "int_qs100.h"
#include "Int_step.h"
#include "stdlib.h"

/**
 * @brief 初始化警告传感器
 *
 */
void App_warning_init(void);

/**
 * @brief 获取警告传感器状态
 *
 * @return uint8_t 0: 无警告 1: 有警告
 */
uint8_t App_warning_get_status(void);

/**
 * @brief 发出告警信息
 *
 */
void App_warning_run(void);

#endif /* __APP_WARNING_H__ */
