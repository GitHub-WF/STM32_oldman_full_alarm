#ifndef __INT_MPU6050_H__
#define __INT_MPU6050_H__

#define MPU6050_I2C_ADDR (0x68 << 1)

#define MPU6050_WHO_AM_I 0x75 // 设备信息
#define MPU605_POWER_MANAGE_1 0x6b // 电源管理1
#define MPU605_POWER_MANAGE_2 0x6c // 电源管理2
#define MPU605_GYRO_FULL_SCALE_RANGE 0x1b // 陀螺仪最大量程
#define MPU605_ACCE_FULL_SCALE_RANGE 0x1c // 加速度最大量程
#define MPU605_INT_ENABLE 0x38 // 中断使能
#define MPU605_USER_CTRL 0x6a // 用户控制
#define MPU605_FIFO_EN 0x23 // FIFO使能
#define MPU605_SAMPLE_RATE_DIV 0x19 // 采样频率
#define MPU605_DLPH_CONFIG 0x1a // 低通滤波器配置

#define MPU6050_GYRO_REG 0x43 // 陀螺仪数据寄存器起始地址
#define MPU6050_ACCE_REG 0x3b // 加速度数据寄存器起始地址
// 数值转换常量值
#define MPU6050_GYRO_SCALE 16.4
#define MPU6050_ACCEL_SCALE 16384.0

#include "i2c.h"
#include "Com_Filter.h"
#include <math.h>

typedef struct
{
  float gx;
  float gy;
  float gz;
} MPU6050_Gyro_t;

typedef struct
{
  float ax;
  float ay;
  float az;
} MPU6050_Accel_t;

/**
 * @brief 初始化MPU6050
 *
 */
void Int_MPU6050_Init(void);

/**
 * @brief 获取陀螺仪数据
 *
 * @param gyro_data
 * @param filter_enable 是否开启低通滤波
 */
void Int_MPU6050_Get_Gyro(MPU6050_Gyro_t *gyro_data, uint8_t filter_enable);

/**
 * @brief 获取加速度数据
 *
 * @param accel_data
 * @param filter_enable 是否开启低通滤波
 */
void Int_MPU6050_Get_Accel(MPU6050_Accel_t *accel_data, uint8_t filter_enable);

#endif /* __INT_MPU6050_H__ */
