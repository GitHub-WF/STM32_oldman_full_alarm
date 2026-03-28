#include "Com_debug.h"

// 重定向方法
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000); // 一次一个字节，大概0.1ms
  return ch;
}
