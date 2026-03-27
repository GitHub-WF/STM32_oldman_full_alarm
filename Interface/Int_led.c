#include "Int_led.h"

// 需要ns级延时函数（主频72MHz，最快时间周期是13.88ns一次tick）
static void Int_led_delay_295ns()
{
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
}

/**
 * @brief 发送 1bit 数据，根据单位时间内高低电平占据时间
 * 0 高电平 0.295s 低电平 0.595s
 * 1 高电平 0.595s 低电平 0.295s
 *
 * @param bit
 */
static void Int_led_send_bit(uint8_t bit)
{
  if (bit == 0)
  {
    // 逻辑时间和实际时间差别较大
    // 1.函数调用耗时：本身需要3-10个时钟周期 => 采用直接寄存器方式
    // 2.需要直接调用nop => 调用个数需要减少
    // 发送0
    GPIOA->ODR |= GPIO_ODR_ODR15;
    Int_led_delay_295ns();
    // 直接写NOP
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    GPIOA->ODR &= ~GPIO_ODR_ODR15;
    Int_led_delay_295ns();
    Int_led_delay_295ns();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
  }
  else
  {
    // 发送1
    GPIOA->ODR |= GPIO_ODR_ODR15;
    Int_led_delay_295ns();
    Int_led_delay_295ns();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    GPIOA->ODR &= ~GPIO_ODR_ODR15;
    Int_led_delay_295ns();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
    // __NOP();__NOP();__NOP();__NOP();__NOP();
  }
}

void Int_led_set(uint32_t data)
{
  for (size_t i = 24; i >= 1; i--)
  {
    Int_led_send_bit((data >> (i - 1)) & 0x01);
  }
}

void Int_led_blink(uint32_t color, uint16_t ms)
{
  Int_led_set(color);
  HAL_Delay(ms);
  Int_led_set(0);
  HAL_Delay(ms);
}

void Int_led_rainbow(uint16_t ms)
{
  // 红橙黄绿蓝靛紫
  // 红 - 红加绿（黄） - 绿 - 绿加蓝（青） - 蓝 - 蓝加红（紫） - 红
  uint32_t color = 0;
  for (uint16_t i = 0; i < 360; i += 5)
  {
    if (i < 60)
    {
      color = ((uint16_t)((i * 1.0 / 60) * 255) << 16) + (255 << 8) + 0; // 红不变 绿加
    }
    else if (i < 120)
    {
      color = (255 << 16) + ((uint16_t)((120.0 - i) / 60 * 255) << 8) + 0; // 绿不变 红减
    }
    else if (i < 180)
    {
      color = (255 << 16) + 0 + (uint16_t)((i - 120.0) / 60 * 255); // 绿不变 蓝加
    }
    else if (i < 240)
    {
      color = ((uint16_t)((240.0 - i) / 60 * 255) << 16) + 0 + 255; // 蓝不变 绿减
    }
    else if (i < 300)
    {
      color = 0 + (((uint16_t)((i - 240.0) / 60 * 255)) << 8) + 255; // 蓝不变 红加
    }
    else if (i < 360)
    {
      color = 0 + (255 << 8) + ((uint16_t)((360.0 - i) / 60 * 255)); // 红不变 蓝减
    }
    Int_led_set(color);
    HAL_Delay(ms);
  }
  Int_led_set(0);
}
