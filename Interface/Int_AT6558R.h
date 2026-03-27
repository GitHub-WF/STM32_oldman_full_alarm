#ifndef __INT_AT6558R_H__
#define __INT_AT6558R_H__

#include "usart.h"
#include "stdlib.h"

#define GPS_FULL_BUFFER_SIZE 512
#define GPS_LINE_BUFFER_SIZE 128

// NMEA语句类型
typedef enum
{
  NMEA_GNGGA,
  NMEA_GNGLL,
  NMEA_GNGSA,
  NMEA_GPGSV,
  NMEA_GNRMC,
  NMEA_GNVTG,
  NMEA_GNZDA,
  NMEA_GPTXT,
  NMEA_UNKNOWN,
} NMEA_Type;

// GPS定位数据结构
typedef struct
{
  double latitude;     // 纬度 (十进制格式)
  char lat_dir;    // 纬度方向
  double longitude;    // 经度 (十进制格式)
  char long_dir;   // 经度方向
  float speed_kph;    // 速度 (千米/小时)
  float hdop;         // 水平定位精度因子
  char time[11];       // GPS时间
  char date[11];       // GPS日期
  uint8_t satellites; // 卫星数量
  uint8_t fix_status;  // 定位状态  0:无效  1:有效
  double timestamp; // 定时数据的时间戳
} GPS_Data;

extern GPS_Data gps_data;

/**
 * @brief 初始化GPS
 *
 */
void Int_GPS_Init(void);

/**
 * @brief 更新GPS数据
 *
 */
void Int_GPS_Update_Data(void);

#endif /* __INT_AT6558R_H__ */
