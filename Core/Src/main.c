/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Int_led.h"
#include "Int_buzzer.h"
#include "App_upload_data.h"
#include "App_warning.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern uint8_t gps_full_buff[GPS_FULL_BUFFER_SIZE];
extern uint16_t gps_full_buff_len;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  // 使用串口输出 -> 一个字节十位（起始位，停止位，8bit数据）-> 115200波特率相当于一秒发送 115200 / 10 = 11520字节，就是1ms 11个字节
  // HAL_UART_Transmit(&huart1, "hello world", 11, 1000); // 耗时1ms 根据波特率115200计算
  // printf("hello world\r\n");
  debug_printf("hello world");

  // 测试单色
  // Int_led_set(LED_RED);

  // 测试闪烁
  /*  for (size_t i = 0; i < 10; i++)
   {
     Int_led_blink(LED_BLUE, 500);
   } */

  // 测试彩虹灯
  /*  for (size_t i = 0; i < 5; i++)
   {
     Int_led_rainbow(15); // 15 * 72 是一轮时间
   } */

  // 测试蜂鸣器
  /* Int_buzzer_on();
  HAL_Delay(1000);
  Int_buzzer_off(); */

  // Int_buzer_play();

  // 计步器测试
  // Int_step_init();

  // GPS测试
  // Int_GPS_Init();

  // 陀螺仪测试
  // Int_MPU6050_Init();

  // IOT芯片测试
  /* Int_QS100_Init();
  uint8_t msg[] = "hello world bro";
  Int_QS100_send_msg(msg); */

  /* Lora测试 */
  /* Int_llcc68_init();

  uint8_t msg[255];
  uint16_t len = 0; */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // uint32_t steps = 0;、
  // 初始化上传数据传感器
  App_upload_data_init();
  // 初始化警告传感器
  App_warning_init();
  uint8_t warning_status = 0;
  uint32_t tick_count = HAL_GetTick();
  while (1)
  {
    /* steps = Int_step_get_count();
    debug_printf("steps = %d", steps); */

    // Int_GPS_Update_Data();

    // HAL_Delay(1000);

    // Int_MPU6050_Get_Gyro(&gyro_data, 1);
    // debug_printf("gyro: %.4f, %.4f, %.4f", gyro_data.gx, gyro_data.gy, gyro_data.gz);
    // Int_MPU6050_Get_Accel(&accel_data, 1);
    // debug_printf("acce: %.4f, %.4f, %.4f", accel_data.ax, accel_data.ay, accel_data.az);
    // HAL_Delay(10);

    // 发送数据
    // Int_llcc68_send("hello world bro", 15);
    // 接收数据
    /* Int_llcc68_receive(msg, &len);
    if (len > 0)
    {
      debug_printf("receive %d bytes", len);
      debug_printf("msg: %s", msg);
      len = 0;
      memset(msg, 0, sizeof(msg));
    } */

    // 判断告警信息
    warning_status = App_warning_get_status();

    // 更新GPS数据

    if (gps_full_buff_len > 0)
    {
      Int_GPS_Update_Data();
      // 清空缓冲区
      memset(gps_full_buff, 0, GPS_FULL_BUFFER_SIZE);
      gps_full_buff_len = 0;

      HAL_UARTEx_ReceiveToIdle_IT(&huart2, gps_full_buff, GPS_FULL_BUFFER_SIZE);
    }

    // 上传数据
    if (warning_status == 1)
    {
      // 3s上传一次
      if (HAL_GetTick() - tick_count >= 3000)
      {
        App_upload_data(warning_status);
        tick_count = HAL_GetTick();
      }
    }
    else
    {
      // 5s上传一次
      if (HAL_GetTick() - tick_count >= 5000)
      {
        App_upload_data(warning_status);
        tick_count = HAL_GetTick();
      }
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
