#include "Int_step.h"

/**
 * @brief 初始化DS3553芯片，设置user_set寄存器
 *
 */
void Int_step_init(void)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); // 拉低片选信号
  HAL_Delay(3);

  uint8_t chip_id = 0;
  HAL_I2C_Mem_Read(&hi2c1, ADDR, CHIP_ID, I2C_MEMADD_SIZE_8BIT, &chip_id, 1, 1000);
  debug_printf("chip_id: %d", chip_id);

  uint8_t byte = 0x1a;
  HAL_I2C_Mem_Write(&hi2c1, ADDR, USER_SET, I2C_MEMADD_SIZE_8BIT, &byte, 1, 1000); // 设置为计步器算法

  HAL_Delay(10);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); // 拉高片选信号
}

/**
 * @brief 获取计步器数值
 *
 * @return uint32_t
 */
uint8_t steps[3] = {0};
uint32_t Int_step_get_count(void)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); // 拉低片选信号
  HAL_Delay(3);

  HAL_I2C_Mem_Read(&hi2c1, ADDR, STEP_CNT_L, I2C_MEMADD_SIZE_8BIT, steps, 3, 1000);

  HAL_Delay(10);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); // 拉高片选信号

  return steps[0] | (steps[1] << 8) | (steps[2] << 16);
}
