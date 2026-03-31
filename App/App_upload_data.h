#ifndef __APP_UPLOAD_DATA_H__
#define __APP_UPLOAD_DATA_H__

#include "Int_AT6558R.h"
#include "Int_MPU6050.h"
#include "Int_step.h"
#include "Int_qs100.h"
#include "Int_llcc68.h"
#include "cJSON.h"

extern MPU6050_Gyro_t gyro_data;
extern MPU6050_Accel_t accel_data;

/**
 * @brief 初始化各传感器
 *
 */
void App_upload_data_init(void);

/**
 * @brief 上传数据
 * 保存各数据：GPS_DATA之外都使用变量保存
 */
void App_upload_data(uint8_t warning_status);

#endif /* __APP_UPLOAD_DATA_H__ */
