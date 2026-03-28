#ifndef __INT_QS100_H__
#define __INT_QS100_H__

#include "usart.h"
#include "stdlib.h"

#define QS100_BUFF_SIZE 512
#define SERVER_IP "112.125.89.8"
#define SERVER_PORT "33712"
#define SEQUENCE_ID 1

typedef enum
{
  IOT_OK = 0,
  IOT_ERROR,
} IOT_STATUS;

/**
 * @brief 初始化QS100
 *
 */
void Int_QS100_Init(void);

/**
 * @brief 查询芯片的附着状态，判断是否能够连接外网
 *
 */
IOT_STATUS Int_QS00_get_ip(void);

/**
 * @brief 创建TCP socket
 *
 * @return IOT_STATUS
 */
IOT_STATUS Int_QS100_open_socket(void);

/**
 * @brief 连接TCP服务器
 *
 * @return IOT_STATUS
 */
IOT_STATUS Int_QS100_connect_tcp(void);

/**
 * @brief 发送TCP消息
 *
 * @param msg 要发送的消息指针
 *
 * @return IOT_STATUS
 */
IOT_STATUS Int_QS100_send(uint8_t *msg);

/**
 * @brief 关闭TCP socket
 *
 * @param socket_link socket链接
 *
 * @return IOT_STATUS
 */
IOT_STATUS Int_QS100_close_socket(uint8_t socket_link);

/**
 * @brief 发送IOT消息到云服务器
 *
 */
void Int_QS100_send_msg(uint8_t *msg);

#endif /* __INT_QS100_H__ */
