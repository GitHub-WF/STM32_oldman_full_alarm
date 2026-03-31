#include "App_upload_data.h"

MPU6050_Gyro_t gyro_data;
MPU6050_Accel_t accel_data = {.az = 16384};

extern uint32_t steps;

void App_upload_data_init(void)
{
  Int_GPS_Init();
  Int_QS100_Init();
  Int_llcc68_init();
}

/**
 * @brief 上传数据
 * 保存各数据：GPS_DATA之外都使用变量保存
 */
void App_upload_data(uint8_t warning_status)
{
  uint32_t uuid = (HAL_GetUIDw2() << 16 | HAL_GetUIDw1() << 8 | HAL_GetUIDw0() << 0);

  HAL_Delay(5);
  Int_MPU6050_Get_Gyro(&gyro_data, 0); // 获取角速度数据
  HAL_Delay(5);
  Int_MPU6050_Get_Accel(&accel_data, 0); // 获取加速度数据
  // HAL_Delay(10)
  // uint32_t steps = Int_step_get_count(); // 获取步数

  // 使用cJson构建上传json数据
  cJSON *json = cJSON_CreateObject();
  // 添加uuid字段
  cJSON_AddNumberToObject(json, "uuid", uuid);
  // 添加warning_status字段
  cJSON_AddNumberToObject(json, "warning_status", warning_status);
  // 添加steps字段
  cJSON_AddNumberToObject(json, "step", steps);
  // 添加陀螺仪字段
  cJSON *gyro_accel = cJSON_CreateObject();
  cJSON_AddItemToObject(json, "gyro_accel", gyro_accel);
  // 角速度
  uint8_t buf[16] = {0};
  sprintf((char *)buf, "%.4f", gyro_data.gx);
  cJSON_AddStringToObject(gyro_accel, "gyro_x", (char *)buf);
  sprintf((char *)buf, "%.4f", gyro_data.gy);
  cJSON_AddStringToObject(gyro_accel, "gyro_y", (char *)buf);
  sprintf((char *)buf, "%.4f", gyro_data.gz);
  cJSON_AddStringToObject(gyro_accel, "gyro_z", (char *)buf);
  // 加速度
  sprintf((char *)buf, "%.4f", accel_data.ax);
  cJSON_AddStringToObject(gyro_accel, "accel_x", (char *)buf);
  sprintf((char *)buf, "%.4f", accel_data.ay);
  cJSON_AddStringToObject(gyro_accel, "accel_y", (char *)buf);
  sprintf((char *)buf, "%.4f", accel_data.az);
  cJSON_AddStringToObject(gyro_accel, "accel_z", (char *)buf);
  // 添加GPS字段（）
  cJSON *gps = cJSON_CreateObject();
  cJSON_AddItemToObject(json, "gps", gps);
  sprintf((char *)buf, "%.4f", gps_data.latitude);
  cJSON_AddStringToObject(gps, "latitude", (char *)buf);
  sprintf((char *)buf, "%.4f", gps_data.longitude);
  cJSON_AddStringToObject(gps, "longitude", (char *)buf);
  cJSON_AddNumberToObject(gps, "speed_kph", gps_data.speed_kph);
  cJSON_AddNumberToObject(gps, "hdop", gps_data.hdop);
  cJSON_AddNumberToObject(gps, "timestamp", gps_data.timestamp);
  cJSON_AddStringToObject(gps, "date", gps_data.date);
  cJSON_AddStringToObject(gps, "time", gps_data.time);
  cJSON_AddNumberToObject(gps, "satellites", gps_data.satellites);
  cJSON_AddNumberToObject(gps, "fix_status", gps_data.fix_status);

  char *json_str = cJSON_PrintUnformatted(json); // 无格式化输出
  // char *json_str = cJSON_Print(json); // 有格式化输出
  if (json_str)
  {
    IOT_STATUS res = Int_QS100_send_msg(json_str, 1);
    if(res == IOT_OK)
    {
      debug_printf("IOT send msg ok");
    }
    else
    {
      debug_printf("IOT send msg failed, switch LoRa");
      uint8_t res = Int_llcc68_send(json_str, strlen(json_str));
      if(res == 0)
      {
        debug_printf("LoRa send msg ok");
      }
      else
      {
        debug_printf("LoRa send msg failed");
      }
    }
    // debug_printf("%s\r\n", json_str);
  }
  HAL_Delay(1000);

  cJSON_free(json_str);
  cJSON_Delete(json);
}
