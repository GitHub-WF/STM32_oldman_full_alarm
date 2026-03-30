#include "Int_llcc68.h"

// 存储在interface文件中添加的函数
static llcc68_handle_t gs_handle;        /**< llcc68 handle */

/**
 * @brief     lora example init
 * @param[in] *callback points to a callback address
 * @return    status code
 *            - 0 success
 *            - 1 init failed
 * @note      none
 */
uint8_t Int_llcc68_init(void)
{
  uint8_t res;
  uint32_t reg;
  uint8_t modulation;
  uint8_t config;

  /* 1. 将interface中的函数添加到gs_handle中 */
  DRIVER_LLCC68_LINK_INIT(&gs_handle, llcc68_handle_t);
  DRIVER_LLCC68_LINK_SPI_INIT(&gs_handle, llcc68_interface_spi_init);
  DRIVER_LLCC68_LINK_SPI_DEINIT(&gs_handle, llcc68_interface_spi_deinit);
  DRIVER_LLCC68_LINK_SPI_WRITE_READ(&gs_handle, llcc68_interface_spi_write_read);
  DRIVER_LLCC68_LINK_RESET_GPIO_INIT(&gs_handle, llcc68_interface_reset_gpio_init);
  DRIVER_LLCC68_LINK_RESET_GPIO_DEINIT(&gs_handle, llcc68_interface_reset_gpio_deinit);
  DRIVER_LLCC68_LINK_RESET_GPIO_WRITE(&gs_handle, llcc68_interface_reset_gpio_write);
  DRIVER_LLCC68_LINK_BUSY_GPIO_INIT(&gs_handle, llcc68_interface_busy_gpio_init);
  DRIVER_LLCC68_LINK_BUSY_GPIO_DEINIT(&gs_handle, llcc68_interface_busy_gpio_deinit);
  DRIVER_LLCC68_LINK_BUSY_GPIO_READ(&gs_handle, llcc68_interface_busy_gpio_read);
  DRIVER_LLCC68_LINK_DELAY_MS(&gs_handle, llcc68_interface_delay_ms);
  DRIVER_LLCC68_LINK_DEBUG_PRINT(&gs_handle, llcc68_interface_debug_print);
  DRIVER_LLCC68_LINK_RECEIVE_CALLBACK(&gs_handle, llcc68_interface_receive_callback);

  /* 2.初始化llcc68模块，检查参数是否完整 */
  res = llcc68_init(&gs_handle);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: init failed.\n");

      return 1;
  }

  /* 3. 进入standby模式，方便后续配置参数，设置时钟源为32MHz */
  res = llcc68_set_standby(&gs_handle, LLCC68_CLOCK_SOURCE_XTAL_32MHZ);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set standby failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 4. 设置停止定时器在前导字上 */
  res = llcc68_set_stop_timer_on_preamble(&gs_handle, LLCC68_LORA_DEFAULT_STOP_TIMER_ON_PREAMBLE);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: stop timer on preamble failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 5. 设置调节器模式为默认值 */
  res = llcc68_set_regulator_mode(&gs_handle, LLCC68_LORA_DEFAULT_REGULATOR_MODE);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set regulator mode failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 6. 设置PA配置，功率配置为最大功率 */
  res = llcc68_set_pa_config(&gs_handle, LLCC68_LORA_DEFAULT_PA_CONFIG_DUTY_CYCLE, LLCC68_LORA_DEFAULT_PA_CONFIG_HP_MAX);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set pa config failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 7. 发送完成之后回退到待机模式 */
  res = llcc68_set_rx_tx_fallback_mode(&gs_handle, LLCC68_RX_TX_FALLBACK_MODE_STDBY_XOSC);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set rx tx fallback mode failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 8. 设置Lora模式 */
  res = llcc68_set_packet_type(&gs_handle, LLCC68_PACKET_TYPE_LORA);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set packet type failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 9. 设置发送参数，功率为最大功率，上功率为最大功率 */
  res = llcc68_set_tx_params(&gs_handle, LLCC68_LORA_DEFAULT_TX_DBM, LLCC68_LORA_DEFAULT_RAMP_TIME);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set tx params failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 10. 设置Lora调制参数，默认值 */
  res = llcc68_set_lora_modulation_params(&gs_handle, LLCC68_LORA_DEFAULT_SF, LLCC68_LORA_DEFAULT_BANDWIDTH,
                                          LLCC68_LORA_DEFAULT_CR, LLCC68_LORA_DEFAULT_LOW_DATA_RATE_OPTIMIZE);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set lora modulation params failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 11. 转换频率为寄存器值 */
  res = llcc68_frequency_convert_to_register(&gs_handle, LLCC68_LORA_DEFAULT_RF_FREQUENCY, (uint32_t *)&reg);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: convert to register failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 12. 设置RF频率 */
  res = llcc68_set_rf_frequency(&gs_handle, reg);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set rf frequency failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 13. 设置缓冲区基地址 */
  res = llcc68_set_buffer_base_address(&gs_handle, 0x00, 0x00);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set buffer base address failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 14. 设置同步超时时间 */
  res = llcc68_set_lora_symb_num_timeout(&gs_handle, LLCC68_LORA_DEFAULT_SYMB_NUM_TIMEOUT);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set lora symb num timeout failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 15. 重置统计信息 */
  res = llcc68_reset_stats(&gs_handle, 0x0000, 0x0000, 0x0000);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: reset stats failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 16. 清除设备错误 */
  res = llcc68_clear_device_errors(&gs_handle);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: clear device errors failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 17. 设置Lora同步字 */
  res = llcc68_set_lora_sync_word(&gs_handle, LLCC68_LORA_DEFAULT_SYNC_WORD);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set lora sync word failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 18. 获取发送调制参数 */
  res = llcc68_get_tx_modulation(&gs_handle, (uint8_t *)&modulation);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: get tx modulation failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }
  modulation |= 0x04;

  /* 19. 设置发送调制参数 */
  res = llcc68_set_tx_modulation(&gs_handle, modulation);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set tx modulation failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 20. 设置接收增益参数 */
  res = llcc68_set_rx_gain(&gs_handle, LLCC68_LORA_DEFAULT_RX_GAIN);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set rx gain failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 21. 设置过电流保护 */
  res = llcc68_set_ocp(&gs_handle, LLCC68_LORA_DEFAULT_OCP);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set ocp failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 22. 获取发送功率限制配置 */
  res = llcc68_get_tx_clamp_config(&gs_handle, (uint8_t *)&config);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: get tx clamp config failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }
  config |= 0x1E;

  /* 23. 设置发送功率限制配置 */
  res = llcc68_set_tx_clamp_config(&gs_handle, config);
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set tx clamp config failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  /* 24. 初始化完成后进入接收模式 */
  res = Int_llcc68_set_receive_mode();
  if (res != 0)
  {
      llcc68_interface_debug_print("llcc68: set receive mode failed.\n");
      (void)llcc68_deinit(&gs_handle);

      return 1;
  }

  return 0;
}

/**
 * @brief  lora example enter to the continuous receive mode
 * @return status code
 *         - 0 success
 *         - 1 enter failed
 * @note   none
 */
uint8_t Int_llcc68_set_receive_mode(void)
{
  /* 设置硬件发送接收开关 */
  HAL_GPIO_WritePin(LORA_TXEN_GPIO_Port, LORA_TXEN_Pin, GPIO_PIN_RESET); // 关闭发送
  HAL_GPIO_WritePin(LORA_RXEN_GPIO_Port, LORA_RXEN_Pin, GPIO_PIN_SET); // 开启接收

  uint8_t setup;

  /* set dio irq */
  if (llcc68_set_dio_irq_params(&gs_handle, LLCC68_IRQ_RX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CRC_ERR | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                LLCC68_IRQ_RX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CRC_ERR | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                0x0000, 0x0000) != 0)
  {
      return 1;
  }

  /* clear irq status */
  if (llcc68_clear_irq_status(&gs_handle, 0x03FFU) != 0)
  {
      return 1;
  }

  /* set lora packet params */
  if (llcc68_set_lora_packet_params(&gs_handle, LLCC68_LORA_DEFAULT_PREAMBLE_LENGTH,
                                    LLCC68_LORA_DEFAULT_HEADER, LLCC68_LORA_DEFAULT_BUFFER_SIZE,
                                    LLCC68_LORA_DEFAULT_CRC_TYPE, LLCC68_LORA_DEFAULT_INVERT_IQ) != 0)
  {
      return 1;
  }

  /* get iq polarity */
  if (llcc68_get_iq_polarity(&gs_handle, (uint8_t *)&setup) != 0)
  {
      return 1;
  }

#if LLCC68_LORA_DEFAULT_INVERT_IQ == LLCC68_BOOL_FALSE
  setup |= 1 << 2;
#else
  setup &= ~(1 << 2);
#endif

  /* set the iq polarity */
  if (llcc68_set_iq_polarity(&gs_handle, setup) != 0)
  {
      return 1;
  }

  /* start receive */
  if (llcc68_continuous_receive(&gs_handle) != 0)
  {
      return 1;
  }

  return 0;
}

/**
 * @brief  lora example enter to the send mode
 * @return status code
 *         - 0 success
 *         - 1 enter failed
 * @note   none
 */
uint8_t Int_llcc68_set_send_mode(void)
{
  /* 设置硬件发送接收开关 */
  HAL_GPIO_WritePin(LORA_TXEN_GPIO_Port, LORA_TXEN_Pin, GPIO_PIN_SET); // 开启发送
  HAL_GPIO_WritePin(LORA_RXEN_GPIO_Port, LORA_RXEN_Pin, GPIO_PIN_RESET); // 关闭接收

  /* set dio irq */
  if (llcc68_set_dio_irq_params(&gs_handle, LLCC68_IRQ_TX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                LLCC68_IRQ_TX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                0x0000, 0x0000) != 0)
  {
      return 1;
  }

  /* clear irq status */
  if (llcc68_clear_irq_status(&gs_handle, 0x03FFU) != 0)
  {
      return 1;
  }

  return 0;
}

/**
 * @brief     lora example send lora data
 * @param[in] *buf points to a data buffer
 * @param[in] len is the data length
 * @return    status code
 *            - 0 success
 *            - 1 send failed
 * @note      none
 */
uint8_t Int_llcc68_send(uint8_t *buf, uint16_t len)
{
  /* 设置发送模式 */
  Int_llcc68_set_send_mode();
  /* send the data */
  if (llcc68_lora_transmit(&gs_handle, LLCC68_CLOCK_SOURCE_XTAL_32MHZ,
                            LLCC68_LORA_DEFAULT_PREAMBLE_LENGTH, LLCC68_LORA_DEFAULT_HEADER,
                            LLCC68_LORA_DEFAULT_CRC_TYPE, LLCC68_LORA_DEFAULT_INVERT_IQ,
                          (uint8_t *)buf, len, 0) != 0)
  {
      return 1;
  }
  /* 设置接收模式 */
  Int_llcc68_set_receive_mode();

  return 0;
}

/**
 * @brief 接收数据
 *
 * @param buf 接收数据缓冲区
 * @param len 接收数据长度
 * @return uint8_t
 */
uint8_t Int_llcc68_receive(uint8_t *buf, uint16_t *len)
{
  /* 接收数据 */
  gs_handle.receive_len = 0;
  llcc68_irq_handler(&gs_handle); // 利用内部调用获取接收数据长度
  if (gs_handle.receive_len != 0)
  {
    memcpy(buf, gs_handle.receive_buf, gs_handle.receive_len);
    *len = gs_handle.receive_len;
    return 0;
  }
  return 1;
}
