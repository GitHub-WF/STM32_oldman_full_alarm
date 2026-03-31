#include "App_warning.h"

uint32_t steps = 0;

MPU6050_Gyro_t warning_gyro_data;
MPU6050_Accel_t warning_accel_data = {.az = 16384};

float gyro_sum[6] = {0.0f};
float accel_sum[6] = {0.0f};
int index = 0;

// 疑似跌倒flag
uint8_t is_fall = 0;
uint32_t last_steps = 0;
uint32_t last_time = 0;

/**
 * @brief 初始化警告传感器
 *
 */
void App_warning_init(void)
{
  Int_step_init();
  Int_MPU6050_Init();

}

/**
 * @brief 获取警告传感器状态
 *
 * @return uint8_t 0: 无警告 1: 有警告
 */
uint8_t App_warning_get_status(void)
{
  HAL_Delay(10);
  Int_MPU6050_Get_Gyro(&warning_gyro_data, 0); // 获取角速度数据
  HAL_Delay(10);
  Int_MPU6050_Get_Accel(&warning_accel_data, 0); // 获取加速度数据
  HAL_Delay(10);
  steps = Int_step_get_count(); // 获取步数

  if (is_fall == 0)
  {
    // 未跌倒，还在平稳状态
    // 角速度绝对值相加是否超过阈值
    gyro_sum[index] = fabsf(warning_gyro_data.gx) + fabsf(warning_gyro_data.gy) + fabsf(warning_gyro_data.gz);
    debug_printf("%f", gyro_sum[index]);
    // 加速度绝对值相加是否超过阈值
    accel_sum[index] = fabsf(warning_accel_data.ax) + fabsf(warning_accel_data.ay) + fabsf(warning_accel_data.az);
    debug_printf("%f", accel_sum[index]);

    debug_printf("%d", steps);

    // 计录5个数据，取平均值
    if (index++ > 4)
    {
      index = 0;
    }

    is_fall = 1;
    for (uint8_t i = 0; i < 3; i++)
    {
      if ((accel_sum[i] < (2.5 * MPU6050_ACCEL_SCALE)) || (gyro_sum[i] < (70 * MPU6050_GYRO_SCALE)))
      {
        is_fall = 0;
      }
    }
    if (is_fall == 1)
    {
      last_steps = steps;
      last_time = HAL_GetTick();
      debug_printf("疑似老人跌倒了");
    }
  }
  else
  {
    if (HAL_GetTick() - last_time > 2000)
    {
      // 检测2s内步数增加范围在0-1之间
      if ((steps - last_steps <= 1))
      {
        // 疑似跌倒
        debug_printf("老人跌倒了");
        App_warning_run();
        return 1;
      }
      else
      {
        // 可能是在跑步
        // debug_printf("老人在跑步");
        is_fall = 0;
        debug_printf("老人在跑步 %d", steps);
      }
    }

  }

  return 0;
}

/**
 * @brief 发出告警信息
 *
 */
void App_warning_run(void)
{
  for (uint8_t i = 0; i < 5; i++)
  {
    Int_led_blink(LED_BLUE, 200);
  }
  Int_buzer_play();
  is_fall = 0;
}
