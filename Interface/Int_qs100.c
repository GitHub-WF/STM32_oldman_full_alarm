#include "Int_qs100.h"

uint8_t qs100_buff[QS100_BUFF_SIZE] = {0};
uint8_t socket_link = 0;
uint8_t is_connecting = 0;
uint16_t get_count = 0;
uint16_t sum_rx_len = 0;
uint8_t count = 0;

// 命令发送前钩子
void Int_qs100_before_send_hook(uint8_t *cmd)
{
  if (strstr((char*)cmd, "AT+RB") != NULL) debug_printf("rebooting...");
  if (strstr((char*)cmd, "AT+CGATT?") != NULL) debug_printf("get ip... %d", get_count++);
  if (strstr((char*)cmd, "AT+NSOCR") != NULL) debug_printf("create socket... %d", get_count++);
  if (strstr((char*)cmd, "AT+NSOCO") != NULL) debug_printf("connect server... %d", get_count++);
  if (strstr((char*)cmd, "AT+SEQUENCE") != NULL) debug_printf("query sequence status... %d", get_count++);
  if (strstr((char*)cmd, "AT+NSOCL") != NULL) debug_printf("close socket... %d", get_count++);
}
// 数据接收完成后钩子
void Int_qs100_after_recv_hook(uint8_t *cmd, uint8_t *data, uint16_t *sum_rx_len)
{
  // 单独判断ATE回显
  if (strstr((char*)cmd, "ATE1") != NULL && strstr((char*)data, "\r\nOK") != NULL)
  {
    // 处理ATE回显
    strcpy((char*)data, "ATE1\r\n\r\nOK\r\n");
    *sum_rx_len = strlen((char*)data);
  }

  data[*sum_rx_len] = '\0';
  debug_printf("%s", data);
}

static void Int_QS100_send_cmd(uint8_t *cmd)
{
  // 1.清空缓冲区
  memset(qs100_buff, 0, QS100_BUFF_SIZE);

  // 2.发送命令
  Int_qs100_before_send_hook(cmd);
  HAL_UART_Transmit(&huart3, (uint8_t *)cmd, strlen((char*)cmd), 1000);

  // 3.接受数据
  sum_rx_len = 0;
  count = 0;
  while (strstr((char *)qs100_buff, "OK") == NULL && strstr((char *)qs100_buff, "ERROR") == NULL)
  {
    // 当没有收到OK或ERROR响应时，继续接收数据，直到接收到OK或ERROR响应，或者接收数据重试超过6次
    uint16_t rx_len = 0;
    HAL_UARTEx_ReceiveToIdle(&huart3, qs100_buff + sum_rx_len, QS100_BUFF_SIZE - sum_rx_len - 1, &rx_len, 1000);
    if ((sum_rx_len + rx_len) > QS100_BUFF_SIZE)
    {
      debug_printf("qs100_buff is full");
      break;
    }

    sum_rx_len += rx_len;
    if (count++ > 6) break;
  }
  Int_qs100_after_recv_hook(cmd, qs100_buff, &sum_rx_len);
}

/**
 * @brief 初始化QS100
 *
 */
void Int_QS100_Init(void)
{
  // 1.硬件唤醒芯片
  HAL_GPIO_WritePin(NB_WKUP_GPIO_Port, NB_WKUP_Pin, GPIO_PIN_SET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(NB_WKUP_GPIO_Port, NB_WKUP_Pin, GPIO_PIN_RESET);
  // 2.软件重启设备
  Int_QS100_send_cmd((uint8_t*)"AT+RB\r\n");
  // 3.设置串口回显
  Int_QS100_send_cmd((uint8_t*)"ATE1\r\n");
  // 4.获取设备信息
  Int_QS100_send_cmd((uint8_t*)"AT+CGMR\r\n");
  // 5.查询SIM卡状态
  Int_QS100_send_cmd((uint8_t*)"AT+CPIN?\r\n");
  // 6.查询信号强度
  Int_QS100_send_cmd((uint8_t*)"AT+CSQ\r\n");
  // 7.查询网络注册状态
  Int_QS100_send_cmd((uint8_t*)"AT+CEREG?\r\n");
  debug_printf("init QS100 ok");
}

/**
 * @brief 查询芯片的附着状态，判断是否能够连接外网
 *
 * @return IOT_STATUS
 */
IOT_STATUS Int_QS00_get_ip(void)
{
  // 发送附着查询命令
  Int_QS100_send_cmd((uint8_t*)"AT+CGATT?\r\n");
  if (strstr((char *)qs100_buff, "+CGATT:1") != NULL)
  {
    get_count = 0;
    return IOT_OK;
  }
  return IOT_ERROR;
}

/**
 * @brief 创建TCP socket
 *
 * @return IOT_STATUS
 */
IOT_STATUS Int_QS100_open_socket(void)
{
  Int_QS100_send_cmd((uint8_t *)"AT+NSOCR=STREAM,6,0,0\r\n");

  if (strstr((char *)qs100_buff, "+NSOCR:") != NULL)
  {
    get_count = 0;
    // 保存使用的 socket 链路号
    char *p = strstr((char *)qs100_buff, "+NSOCR:");
    if (p != NULL)
    {
      p += 7;
      socket_link = atoi(p);
    }
    debug_printf("socket_link: %d", socket_link);
    return IOT_OK;
  }
  return IOT_ERROR;
}

/**
 * @brief 连接TCP服务器
 *
 * @return IOT_STATUS
 */
char cmd_buff[128];
IOT_STATUS Int_QS100_connect_tcp(void)
{
  // 清空缓冲区
  memset(cmd_buff, 0, sizeof(cmd_buff));
  sprintf(cmd_buff, "AT+NSOCO=%d,%s,%s\r\n", socket_link, SERVER_IP, SERVER_PORT);
  Int_QS100_send_cmd((uint8_t *)cmd_buff);
  if (strstr((char *)qs100_buff, "OK") != NULL)
  {
    get_count = 0;
    debug_printf("connect tcp success");
    return IOT_OK;
  }
  return IOT_ERROR;
}

/**
 * @brief 发送TCP消息
 *
 * @param msg 要发送的消息指针
 *
 * @return IOT_STATUS
 */
uint8_t msg_len = 0;
uint8_t msg_hex[QS100_BUFF_SIZE] = {0}; // 单次数据最大长度为 1024 / 2 字节
IOT_STATUS Int_QS100_send(char *msg)
{
  // 清空缓冲区
  memset(cmd_buff, 0, sizeof(cmd_buff));
  memset(msg_hex, 0, sizeof(msg_hex));

  msg_len = strlen(msg);
  // 将msg转换成16进制字符串
  for (uint16_t i = 0; i < msg_len; i++)
  {
    sprintf((char *)msg_hex + i * 2, "%02X", msg[i]);
  }
  sprintf(cmd_buff, "AT+NSOSD=%d,%d,%s,0x200,%d\r\n", socket_link, msg_len, msg_hex, SEQUENCE_ID);
  Int_QS100_send_cmd((uint8_t *)cmd_buff);
  if (strstr((char *)qs100_buff, "OK") != NULL)
  {
    memset(cmd_buff, 0, sizeof(cmd_buff));
    sprintf(cmd_buff, "AT+SEQUENCE=%d,%d\r\n", socket_link, SEQUENCE_ID);
    uint8_t status = 0;
    while (1)
    {
      // 判断消息是否发送成功
      Int_QS100_send_cmd((uint8_t *)cmd_buff);

      if (strstr((char *)qs100_buff, "OK") != NULL)
      {
        char *p = strstr((char *)qs100_buff, "OK");
        if (p != NULL)
        {
          while (p > (char *)qs100_buff)
          {
            p--;
            // 找到第一个数字字符就停止
            if (*p >= '0' && *p <= '9')
            {
              break;
            }
          }
          status = atoi(p);
        }
        if (status == 1)
        {
          // 判断±1
          if (strstr((char *)qs100_buff, "-") == NULL)
          {
            get_count = 0;
            debug_printf("send info success");
            return IOT_OK;
          }
          else
          {
            debug_printf("unuse sequence id");
            break;
          }
        }
        else if (status == 0)
        {
          debug_printf("send info failed");
          break;
        }
        else if (status == 2)
        {
          debug_printf("info is sending...");
        }
      }
      HAL_Delay(1000);
    }
  }
  return IOT_ERROR;
}

/**
 * @brief 关闭TCP socket
 *
 * @param socket_link socket链接
 *
 * @return IOT_STATUS
 */
IOT_STATUS Int_QS100_close_socket(uint8_t socket_link)
{
  // 关闭TCP socket
  sprintf(cmd_buff, "AT+NSOCL=%d\r\n", socket_link);
  Int_QS100_send_cmd((uint8_t *)cmd_buff);
  if (strstr((char *)qs100_buff, "OK") != NULL)
  {
    get_count = 0;
    debug_printf("close tcp success");
    return IOT_OK;
  }
  return IOT_ERROR;
}

/**
 * @brief 发送IOT消息
 *
 */
// 重试次数
uint8_t retry_count = 0;
IOT_STATUS Int_QS100_send_msg(char *msg, uint8_t still_data)
{
  if (is_connecting == 0)
  {
    // 等待附着状态为1
    retry_count = 0;
    while (Int_QS00_get_ip() != IOT_OK && retry_count < 3)
    {
      retry_count++;
      if(retry_count >= 3)
      {
        debug_printf("wait ip failed");
        return IOT_ERROR;
      }
      HAL_Delay(1000);
    }
    // 创建TCP socket
    while (Int_QS100_open_socket() != IOT_OK)
    {
      HAL_Delay(1000);
    }
    // TCP连接
    retry_count = 0;
    while (Int_QS100_connect_tcp() != IOT_OK && retry_count < 3)
    {
      retry_count++;
      if (retry_count >= 3)
      {
        debug_printf("connect tcp failed");
        return IOT_ERROR;
      }

      HAL_Delay(1000);
    }
    is_connecting = 1;
  }
  // 发送TCP消息
  while (Int_QS100_send(msg) != IOT_OK)
  {
    HAL_Delay(1000);
  }
  if (still_data == 0)
  {
    // 关闭TCP连接
    while (Int_QS100_close_socket(socket_link) != IOT_OK)
    {
      HAL_Delay(1000);
    }
    is_connecting = 0;
  }
  return IOT_OK;
}
