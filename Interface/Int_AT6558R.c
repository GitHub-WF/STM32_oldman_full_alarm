#include "Int_AT6558R.h"

uint8_t gps_full_buff[GPS_FULL_BUFFER_SIZE] = {0};
uint16_t gps_full_buff_len = 0;

uint8_t gps_line_buff[GPS_LINE_BUFFER_SIZE] = {0};
uint16_t gps_line_buff_len = 0;

GPS_Data gps_data = {0};

/**
 * @brief 初始化GPS
 *
 */
void Int_GPS_Init(void)
{
  // 打开GPS
  HAL_GPIO_WritePin(GPS_EN_GPIO_Port, GPS_EN_Pin, GPIO_PIN_SET);

  HAL_Delay(100);
}
/**
 * @brief 数据校验
 *
 * @param buff
 * @return uint8_t 0: 校验失败 1：校验成功
 */
uint8_t GPS_Check_Sum(char *line_buff)
{
  char *end_flag = strstr(line_buff, "*");
  if (line_buff[0] != '$' || end_flag == NULL) return 0;

  uint8_t check_sum = 0;
  for (int i = 1; i < end_flag - line_buff; i++)
  {
    check_sum ^= line_buff[i];
  }
  return check_sum == strtol(end_flag + 1, NULL, 16);
}

/**
 * @brief 判断当前行数据类型
 *
 * @param buff
 * @return NMEA_Type
 */
NMEA_Type GPS_Get_Type(char *buff)
{
  NMEA_Type nmea_type = NMEA_UNKNOWN;

  if (strstr(buff, "GGA")) nmea_type = NMEA_GNGGA;
  if (strstr(buff, "GLL")) nmea_type = NMEA_GNGLL;
  if (strstr(buff, "GSA")) nmea_type = NMEA_GNGSA;
  if (strstr(buff, "GSV")) nmea_type = NMEA_GPGSV;
  if (strstr(buff, "RMC")) nmea_type = NMEA_GNRMC;
  if (strstr(buff, "VTG")) nmea_type = NMEA_GNVTG;
  if (strstr(buff, "ZDA")) nmea_type = NMEA_GNZDA;
  if (strstr(buff, "TXT")) nmea_type = NMEA_GPTXT;

  return nmea_type;
}

/**
 * @brief 更新GPS数据
 *
 */

void Int_GPS_Update_Data(void)
{
  // 清空缓冲区
  memset(gps_full_buff, 0, GPS_FULL_BUFFER_SIZE);
  gps_full_buff_len = 0;
  // GPS芯片发送完当次定位数据之后，会发送空闲帧数据，当做接受完整数据的标志
  // HAL_UARTEx_ReceiveToIdle 是阻塞式接受，以下情况会跳过进入下一次执行
  // 1. 达到超时时间或缓冲区接受满了 => GPS芯片未正常工作或缓冲区太小
  // 2. 数据接收到了空闲帧
  HAL_UARTEx_ReceiveToIdle(&huart2, gps_full_buff, GPS_FULL_BUFFER_SIZE, &gps_full_buff_len, 1200);
  if (gps_full_buff_len > 0)
  {

    // 处理数据
    debug_printf("gps info: %s", gps_full_buff);
    char *buff_p = (char *)gps_full_buff;
    // char *buff_p = "$GNGGA,081621.000,3106.67691,N,12113.52666,E,1,11,2.9,36.9,M,10.3,M,,*71\r\n$GNRMC,081621.000,A,3106.67691,N,12113.52666,E,0.00,0.00,040126,,,A,V*0D\r\n$GNVTG,0.00,T,,M,0.00,N,5.00,K,A*26\r\n";

    while (1)
    {
      // 1.寻找一行的开始和结尾
      char *start_p = strstr(buff_p, "$");
      char *end_p = strstr(buff_p, "\r\n");
      if (start_p == NULL || end_p == NULL) break;
      // 2.保存当前行数据
      gps_line_buff_len = end_p - start_p + 1;
      if (gps_line_buff_len > GPS_LINE_BUFFER_SIZE) gps_line_buff_len = GPS_LINE_BUFFER_SIZE;
      memset(gps_line_buff, 0, GPS_LINE_BUFFER_SIZE);
      memcpy(gps_line_buff, start_p, gps_line_buff_len);

      // 3.判断当前行数据类型
      // 先进行校验
      if (GPS_Check_Sum((char *)gps_line_buff))
      {
        // 根据逗号切割数据
        char single_data[20][20] = {0};
        char *p = (char *)strtok((char *)gps_line_buff, ",");
        int i = 0;
        while(p != NULL)
        {
          // 把切割出来的内容 → 存进数组
          // 判断最后一个数据是否带*
          if (strstr(p, "*"))
          {
            // 去掉*及其后面的内容
            strncpy(single_data[i], p, strstr(p, "*") - p);
          }
          else
          {
            strcpy(single_data[i], p);
          }
          p = strtok(NULL, ",");
          i++;
        }
        NMEA_Type nmea_type = GPS_Get_Type((char *)gps_line_buff);
        switch (nmea_type)
        {
          case NMEA_GNGGA: {
            gps_data.satellites = (uint8_t)strtol(single_data[7], NULL, 10);
            gps_data.hdop = strtod(single_data[8], NULL);
            gps_data.timestamp = (double)strtod(single_data[1], NULL);
            break;
          }
          case NMEA_GNRMC: {
            // 纬度
            if (single_data[2][0] != 'V')
            {
              double dmm = strtod(single_data[3], NULL);
              double deg = (int)(dmm / 100);
              double min = dmm - deg * 100;
              gps_data.latitude = deg + min / 60.0;
              gps_data.lat_dir = single_data[4][0];
              // 经度
              dmm = strtod(single_data[5], NULL);
              deg = (int)(dmm / 100);
              min = dmm - deg * 100;
              gps_data.longitude = deg + min / 60.0;
              gps_data.long_dir = single_data[6][0];

              gps_data.fix_status = (single_data[2][0] == 'A') ? 1 : 0;
            }
            else
            {
              debug_printf("GPS RMC data error");
            }
            break;
          }
          case NMEA_GNVTG: {
            gps_data.speed_kph = strtod(single_data[7], NULL);
            break;
          }
          case NMEA_GNZDA: {
            debug_printf("zda");
            strcpy(gps_data.time, single_data[1]);
            memset(gps_data.date, 0, sizeof(gps_data.date));
            sprintf((char *)gps_data.date, "%s%s%s",
              (char *)single_data[4],
              (char *)single_data[3],
              (char *)single_data[2]);
            break;
          }
          default:
            break;
        }
      }
      else
      {
        debug_printf("GPS data check sum error");
      }
      buff_p = end_p + 2;
    }
  }
}
