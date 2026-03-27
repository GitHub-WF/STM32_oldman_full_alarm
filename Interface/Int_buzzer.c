#include "Int_buzzer.h"

/**
 * @brief 开启蜂鸣器
 *
 */
void Int_buzzer_on(void)
{
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

/**
 * @brief 关闭蜂鸣器
 *
 */
void Int_buzzer_off(void)
{
  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}

/**
 * @brief 设置蜂鸣器频率
 *
 */
void Int_buzzer_set(uint16_t freq)
{
  // 预分频过后的频率是10000Hz，再经过重装载系数获得最终频率
  __HAL_TIM_SetAutoreload(&htim3, 10000 / freq);
  // 修改占空比
  __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 5000 / freq);
  // 清零
  __HAL_TIM_SetCounter(&htim3, 0);
}

/**
 * @brief 播放音乐
 *
 */
void Int_buzer_play(void)
{
  uint8_t music[] = {1,1,5,5,6,6,5,0,4,4,3,3,2,2,1,0};
  for (size_t i = 0; i < sizeof(music) / music[0]; i++)
  {
    debug_printf("freq = %d", music[i]);

    if (music[i] == 0) continue;

    switch (music[i])
    {
      case 1:
        Int_buzzer_set(BUZZER_FREQ_1);
        break;
      case 2:
        Int_buzzer_set(BUZZER_FREQ_2);
        break;
      case 3:
        Int_buzzer_set(BUZZER_FREQ_3);
        break;
      case 4:
        Int_buzzer_set(BUZZER_FREQ_4);
        break;
      case 5:
        Int_buzzer_set(BUZZER_FREQ_5);
        break;
      case 6:
        Int_buzzer_set(BUZZER_FREQ_6);
        break;
    }
    Int_buzzer_on();
    HAL_Delay(music[i + 1] == 0 ? 800 : 400);
    Int_buzzer_off();
    HAL_Delay(music[i + 1] == 0 ? 200 : 100);
  }
}
