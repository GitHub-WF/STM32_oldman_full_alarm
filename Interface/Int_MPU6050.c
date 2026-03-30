#include "Int_MPU6050.h"

uint8_t Int_Mpu6050_Read_Reg(uint8_t reg)
{
  uint8_t data = 0;
  HAL_I2C_Mem_Read(&hi2c1, MPU6050_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
  return data;
}
void Int_Mpu6050_Write_Reg(uint8_t reg, uint8_t data)
{
  HAL_I2C_Mem_Write(&hi2c1, MPU6050_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
}

MPU6050_Gyro_t cali_gyro;
MPU6050_Accel_t cali_accel;
void Int_MPU6050_Calibrate(void)
{
  // 1.判断 角速度/角速度 是否稳定（静止状态）
  MPU6050_Gyro_t temp_gyro = {0};
  MPU6050_Accel_t temp_accel = {0};
  uint8_t count = 0;
  MPU6050_Gyro_t sum_50_gyro = {0};
  MPU6050_Accel_t sum_50_accel = {0};
  while (1)
  {
    Int_MPU6050_Get_Gyro(&temp_gyro, 0);
    Int_MPU6050_Get_Accel(&temp_accel, 0);
    if (fabs(temp_gyro.gx) < 300 && fabs(temp_gyro.gy) < 300 && fabs(temp_gyro.gz) < 300)
    {
      if (count++ > 50) break;

      // 角速度50次累加值
      sum_50_gyro.gx += temp_gyro.gx;
      sum_50_gyro.gy += temp_gyro.gy;
      sum_50_gyro.gz += temp_gyro.gz;
      // 加速度50次累加值
      sum_50_accel.ax += temp_accel.ax;
      sum_50_accel.ay += temp_accel.ay;
      sum_50_accel.az += (temp_accel.az - 16384);

      HAL_Delay(10);
    }
  }
  // 2.处于静止状态则用获取的 角速度/加速度 值减去50次平均漂移数据
  cali_gyro.gx = sum_50_gyro.gx / 50;
  cali_gyro.gy = sum_50_gyro.gy / 50;
  cali_gyro.gz = sum_50_gyro.gz / 50;

  cali_accel.ax = sum_50_accel.ax / 50;
  cali_accel.ay = sum_50_accel.ay / 50;
  cali_accel.az = sum_50_accel.az / 50;
}
/**
 * @brief 初始化MPU6050
 *
 */
void Int_MPU6050_Init(void)
{
  /* uint8_t buff = Int_Mpu6050_Read_Reg(MPU6050_WHO_AM_I);
  debug_printf("data: %#x", buff); */

  // 1.先将寄存器复位，然后从睡眠模式唤醒
  Int_Mpu6050_Write_Reg(MPU605_POWER_MANAGE_1, 0x80); // 复位
  HAL_Delay(100);
  // 2.等待复位成功
  uint8_t data = Int_Mpu6050_Read_Reg(MPU605_POWER_MANAGE_1);
  while (data != 0x40)
  {
    data = Int_Mpu6050_Read_Reg(MPU605_POWER_MANAGE_1);
  }
  // 3.唤醒陀螺仪，同时设置方式1为时钟源
  Int_Mpu6050_Write_Reg(MPU605_POWER_MANAGE_1, 0x01); // 唤醒

  // 4.设置角速度传感器和加速度传感器满量程范围（±2000°/s，±2g）
  Int_Mpu6050_Write_Reg(MPU605_GYRO_FULL_SCALE_RANGE, 0x18);
  Int_Mpu6050_Write_Reg(MPU605_ACCE_FULL_SCALE_RANGE, 0x00);

  // 6.关闭所有中断
  Int_Mpu6050_Write_Reg(MPU605_INT_ENABLE, 0x00);

  // 7.用户控制寄存器关闭
  Int_Mpu6050_Write_Reg(MPU605_USER_CTRL, 0x00);

  // 8.FIFO使能
  Int_Mpu6050_Write_Reg(MPU605_FIFO_EN, 0x00);

  // 9.设置采样频率 200Hz
  Int_Mpu6050_Write_Reg(MPU605_SAMPLE_RATE_DIV, 0x04);

  // 10.配置低通滤波器（建议设置为带宽1/2）
  Int_Mpu6050_Write_Reg(MPU605_DLPH_CONFIG, 0x02);

  // 11.使能加速度传感器和角速度传感器
  Int_Mpu6050_Write_Reg(MPU605_POWER_MANAGE_2, 0x00);

  // 12.零点误差处理
  Int_MPU6050_Calibrate();

  debug_printf("init ok");
}

/**
 * @brief 获取陀螺仪数据（对角速度 抖动不大 使用一阶低通滤波）
 *
 * @param gyro_data
 */
void Int_MPU6050_Get_Gyro(MPU6050_Gyro_t *gyro_data, uint8_t filter_enable)
{
  uint8_t buff[6] = {0};
  if (HAL_I2C_Mem_Read(&hi2c1, MPU6050_I2C_ADDR, MPU6050_GYRO_REG, I2C_MEMADD_SIZE_8BIT, buff, 6, 1000)== HAL_OK)
  {
    MPU6050_Gyro_t temp_gyro = {0};
    temp_gyro.gx = (int16_t)((buff[0] << 8) | buff[1]) - cali_gyro.gx;
    temp_gyro.gy = (int16_t)((buff[2] << 8) | buff[3]) - cali_gyro.gy;
    temp_gyro.gz = (int16_t)((buff[4] << 8) | buff[5]) - cali_gyro.gz;

    // 对结果数据进低通滤波（低通滤波的第一个测量值最好是0，稳定之后再使用）
    if (filter_enable) // 判断是否开启低通滤波
    {
      gyro_data->gx = Com_Filter_LowPass(temp_gyro.gx, gyro_data->gx);
      gyro_data->gy = Com_Filter_LowPass(temp_gyro.gy, gyro_data->gy);
      gyro_data->gz = Com_Filter_LowPass(temp_gyro.gz, gyro_data->gz);
    }
    else
    {
      gyro_data->gx = temp_gyro.gx;
      gyro_data->gy = temp_gyro.gy;
      gyro_data->gz = temp_gyro.gz;
    }
  }
  else
  {
    debug_printf("I2C read error, restart...");
    // I2C 死锁恢复
    HAL_I2C_DeInit(&hi2c1);
    HAL_I2C_Init(&hi2c1);
    HAL_Delay(5);
  }
}

/**
 * @brief 获取加速度数据（卡尔曼滤波，用于毛刺更多，抖动更大的数据）
 *
 * @param accel_data
 */
void Int_MPU6050_Get_Accel(MPU6050_Accel_t *accel_data, uint8_t filter_enable)
{
  uint8_t buff[6] = {0};
  if (HAL_I2C_Mem_Read(&hi2c1, MPU6050_I2C_ADDR, MPU6050_ACCE_REG, I2C_MEMADD_SIZE_8BIT, buff, 6, 1000) == HAL_OK)
  {
    MPU6050_Accel_t temp_accel = {0};
    temp_accel.ax = (int16_t)((buff[0] << 8) | buff[1]) - cali_accel.ax;
    temp_accel.ay = (int16_t)((buff[2] << 8) | buff[3]) - cali_accel.ay;
    temp_accel.az = (int16_t)((buff[4] << 8) | buff[5]) - cali_accel.az;

    if (filter_enable)
    {
      accel_data->ax = Com_Filter_KalmanFilter(&kfs[0], temp_accel.ax);
      accel_data->ay = Com_Filter_KalmanFilter(&kfs[1], temp_accel.ay);
      accel_data->az = Com_Filter_KalmanFilter(&kfs[2], temp_accel.az);
    }
    else
    {
      accel_data->ax = temp_accel.ax;
      accel_data->ay = temp_accel.ay;
      accel_data->az = temp_accel.az;
    }
  }
  else
  {
    debug_printf("I2C read error, restart...");
    // I2C 死锁恢复
    HAL_I2C_DeInit(&hi2c1);
    HAL_I2C_Init(&hi2c1);
    HAL_Delay(5);
  }
}
