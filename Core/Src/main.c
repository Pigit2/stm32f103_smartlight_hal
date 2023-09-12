/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint8_t UART1_RX_TEMP[1];                                                 // UART1 RECIEVE ONE CHARACTER TEMP
uint8_t UART1_RX_CNT = 0;                                                 // UART1 RECIEVE CHARACTER COUNT
char UART1_RX_BUF[256];                                                   // UART1 RECIEVE STRING
char SUB_RX_BUF[256];                                                     // TEMP ACCESS AND PROCESS BY UART1_RX_BUF
char *pcTEMP_RX;                                                          // SAVE "strstr" STRING BY UART1_RX_BUF
char pcWEEK[4];                                                           // WEEK RECIEVE BY THE UART1
uint16_t usYEAR_FULL, usYEAR_TAIL, usMONTH, usDATE, usHOUR, usMIN, usSEC; // TIME RECIEVE BY THE UART1,
                                                                          // FULL: FOUR DIGITS; TAIL: DOUBLE DIGITS
uint16_t usTIMING_HOUR, usTIMING_MIN, usTIMING_SEC;                       // TIMING RECIEVE BY THE UART1
uint16_t usCTR_BRIGHTNESS;                                                // SET THE BRIGHTNESS BY UART1 COMMAND
char pcREQUEST_ID[37];                                                    // SAVE THE IOT COMMAND REQUEST_ID
uint8_t usLOWER_HALF_SFLAG = 0;                                           // LOWER DISPLAY STATUS FLAG
                                                                          // 0: POWER; 1: TIMING; 2: VOLTAGE; 3: CURRENT
uint8_t usTIMING_CMD;                                                     // TIMING COMMAND, 1: TIMING ON; 0: TIMING OFF
uint16_t psBRIGHTNESS_STORAGE[3] = {0};                                   // USE FOR SAVE BRIGHTNESS TO UPLOAD
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_TIM4_Init();
  MX_ADC1_Init();
  MX_RTC_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();                                               // OLED INITITAL
	HAL_TIM_Base_Start(&htim3);                                // START TIM3 TO TRIGGER ADC1
  ADC_GET_START();                                           // ENABLE DMA TO ADC1
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);                  // ENABLE PWM CHANNEL1 OUTPUT
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);                  // ENABLE PWM CHANNEL2 OUTPUT
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);                  // ENABLE PWM CHANNEL4 OUTPUT
	HAL_UART_Receive_IT(&huart1, (uint8_t *)UART1_RX_TEMP, 1); // ENABLE RECEIVE IT ONCE
	HuaweiIot_init();                                          // CONNECT WIFI AND HUAWEIIOT
	HAL_Delay(2000);
	RTC_SYNC_TIME();                                           // WAIT RTC GET THE NEW TIME
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();
  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV128;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
 * @description: USART1 IT RECEIVE CALLBACK FUNCTION
 * @param {UART_HandleTypeDef} *huart
 * @return {*}
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  
  if(huart->Instance == USART1)
  {
		if(UART1_RX_TEMP[0] == ' ' && UART1_RX_CNT == 0)
		{
			//delete space
		}
		else
		{
			UART1_RX_BUF[UART1_RX_CNT] = UART1_RX_TEMP[0];
			UART1_RX_CNT++;
		}
    
    if(UART1_RX_TEMP[0] == 0x0A)
    {
			//do something, receive a string finish
			if(UART1_RX_CNT >= 2 && UART1_RX_CNT <= 6) //wait for check AT status,wait for test strncpy length
      {
        strncpy(SUB_RX_BUF, UART1_RX_BUF, 2);
        if(strcmp(SUB_RX_BUF, "OK") == 0)
          atok_rec_flag = 1;
        strncpy(SUB_RX_BUF, UART1_RX_BUF, 5);
        if(strcmp(SUB_RX_BUF, "ready") == 0)
          atok_rec_flag = 1;
        strncpy(SUB_RX_BUF, UART1_RX_BUF, 5);
        if(strcmp(SUB_RX_BUF, "ERROR") == 0)
          atok_rec_flag = 0;
        // memset(SUB_RX_BUF, 0, sizeof(SUB_RX_BUF));//wait for check can delete
      }
      else
      {
        strncpy(SUB_RX_BUF, UART1_RX_BUF, 12);
        //GET TIME
        if(strcmp(SUB_RX_BUF, "+CIPSNTPTIME") == 0)
        {
          if(UART1_RX_BUF[22] == ' ') //WHEN THE DATE IS SINGLE,DELETE THE SPACE
          {
            usYEAR_FULL = (UART1_RX_BUF[32]-'0')*1000+(UART1_RX_BUF[33]-'0')*100+(UART1_RX_BUF[34]-'0')*10+(UART1_RX_BUF[35]-'0');
            usYEAR_TAIL = (UART1_RX_BUF[34]-'0')*10+(UART1_RX_BUF[35]-'0');
            usDATE = (UART1_RX_BUF[21]-'0');
            usHOUR = (UART1_RX_BUF[23]-'0')*10+(UART1_RX_BUF[24]-'0');
            usMIN  = (UART1_RX_BUF[26]-'0')*10+(UART1_RX_BUF[27]-'0');
            usSEC  = (UART1_RX_BUF[29]-'0')*10+(UART1_RX_BUF[30]-'0');
          }
          else
          {
            usYEAR_FULL = (UART1_RX_BUF[33]-'0')*1000+(UART1_RX_BUF[34]-'0')*100+(UART1_RX_BUF[35]-'0')*10+(UART1_RX_BUF[36]-'0');
            usYEAR_TAIL = (UART1_RX_BUF[35]-'0')*10+(UART1_RX_BUF[36]-'0');
            usDATE = (UART1_RX_BUF[21]-'0')*10+(UART1_RX_BUF[22]-'0');
            usHOUR = (UART1_RX_BUF[24]-'0')*10+(UART1_RX_BUF[25]-'0');
            usMIN  = (UART1_RX_BUF[27]-'0')*10+(UART1_RX_BUF[28]-'0');
            usSEC  = (UART1_RX_BUF[30]-'0')*10+(UART1_RX_BUF[31]-'0');
          }
          if(usYEAR_FULL < 2023)
          {
            OLED_Clear();
            OLED_ShowString(0,0,(u8 *)"Wait RTC...",16);
            RTC_SYNC_FLAG = 1;
          }
          else
          {
            //SYNC FINISH
            RTC_SYNC_FLAG = 0;
            strncpy(pcWEEK, UART1_RX_BUF+13, 3);
            if(UART1_RX_BUF[17] == 'J' && UART1_RX_BUF[18] == 'a' && UART1_RX_BUF[19] == 'n')
              usMONTH = 1;
            else if(UART1_RX_BUF[17] == 'F' && UART1_RX_BUF[18] == 'e' && UART1_RX_BUF[19] == 'b')
              usMONTH = 2;
            else if(UART1_RX_BUF[17] == 'M' && UART1_RX_BUF[18] == 'a' && UART1_RX_BUF[19] == 'r')
              usMONTH = 3;
            else if(UART1_RX_BUF[17] == 'A' && UART1_RX_BUF[18] == 'p' && UART1_RX_BUF[19] == 'r')
              usMONTH = 4;
            else if(UART1_RX_BUF[17] == 'M' && UART1_RX_BUF[18] == 'a' && UART1_RX_BUF[19] == 'y')
              usMONTH = 5;
            else if(UART1_RX_BUF[17] == 'J' && UART1_RX_BUF[18] == 'u' && UART1_RX_BUF[19] == 'n')
              usMONTH = 6;
            else if(UART1_RX_BUF[17] == 'J' && UART1_RX_BUF[18] == 'u' && UART1_RX_BUF[19] == 'l')
              usMONTH = 7;
            else if(UART1_RX_BUF[17] == 'A' && UART1_RX_BUF[18] == 'u' && UART1_RX_BUF[19] == 'g')
              usMONTH = 8;
            else if(UART1_RX_BUF[17] == 'S' && UART1_RX_BUF[18] == 'e' && UART1_RX_BUF[19] == 'p')
              usMONTH = 9;
            else if(UART1_RX_BUF[17] == 'O' && UART1_RX_BUF[18] == 'c' && UART1_RX_BUF[19] == 't')
              usMONTH = 10;
            else if(UART1_RX_BUF[17] == 'N' && UART1_RX_BUF[18] == 'o' && UART1_RX_BUF[19] == 'v')
              usMONTH = 11;
            else if(UART1_RX_BUF[17] == 'D' && UART1_RX_BUF[18] == 'e' && UART1_RX_BUF[19] == 'c')
              usMONTH = 12;
            
            //printf("test:%d/%d/%d,%d:%d:%d,%s\r\n",usYEAR_FULL,usMONTH,usDATE,usHOUR,usMIN,usSEC,pcWEEK);
            RTC_SET(usYEAR_TAIL,usMONTH,usDATE,usHOUR,usMIN,usSEC);
          }
        }
        //GET MQTT COMMAND
        else if(strcmp(SUB_RX_BUF, "+MQTTSUBRECV") == 0)
        {
          pcTEMP_RX = strstr(UART1_RX_BUF, "request_id");
          if(pcTEMP_RX != NULL)
          {
            strncpy(pcREQUEST_ID, &pcTEMP_RX[10], 37);
            printf("r_id:%s\r\n",pcREQUEST_ID); // FIND THE REQUEST ID
            pcTEMP_RX = strstr(UART1_RX_BUF, "power");
            if(pcTEMP_RX != NULL)
            {
              if(pcTEMP_RX[8] == 'O' && pcTEMP_RX[9] == 'N')
              {
                //TIMING ON COMMAND
                usTIMING_HOUR = (pcTEMP_RX[19]-'0')*10+(pcTEMP_RX[20]-'0');
                usTIMING_MIN  = (pcTEMP_RX[21]-'0')*10+(pcTEMP_RX[22]-'0');
                usTIMING_SEC  = (pcTEMP_RX[23]-'0')*10+(pcTEMP_RX[24]-'0');
                //printf("ON:%d:%d:%d\r\n",usTIMING_HOUR,usTIMING_MIN,usTIMING_SEC);//TIMING ON
                RTC_SET_ALARM(usTIMING_HOUR, usTIMING_MIN, usTIMING_SEC);
                usLOWER_HALF_SFLAG = 1;
                usTIMING_CMD = 1;
                OLED_ShowString(0,6,(u8 *)"              ",16); //CLEAN LOWER HALF
              }
              else if (pcTEMP_RX[8] == 'O' && pcTEMP_RX[9] == 'F' && pcTEMP_RX[10] == 'F')
              {
                //TIMING OFF COMMAND
                usTIMING_HOUR = (pcTEMP_RX[20]-'0')*10+(pcTEMP_RX[21]-'0');
                usTIMING_MIN  = (pcTEMP_RX[22]-'0')*10+(pcTEMP_RX[23]-'0');
                usTIMING_SEC  = (pcTEMP_RX[24]-'0')*10+(pcTEMP_RX[25]-'0');
                //printf("OFF:%d:%d:%d\r\n",usTIMING_HOUR,usTIMING_MIN,usTIMING_SEC);//TIMING OFF
                RTC_SET_ALARM(usTIMING_HOUR, usTIMING_MIN, usTIMING_SEC);
                usLOWER_HALF_SFLAG = 1;
                OLED_ShowString(0,6,(u8 *)"              ",16); //CLEAN LOWER HALF
              }
              else
              {
                //SET LIGHT COMMAND
                usCTR_BRIGHTNESS = (pcTEMP_RX[8]-'0')*100+(pcTEMP_RX[9]-'0')*10;
                if(pcTEMP_RX[23] == 'A' && pcTEMP_RX[24] == 'L' && pcTEMP_RX[25] == 'L')
                {
                  //SET ALL IN LIGHT
                  //printf("ALL:%d\r\n",usCTR_BRIGHTNESS);
                  psBRIGHTNESS_STORAGE[0] = usCTR_BRIGHTNESS;
                  psBRIGHTNESS_STORAGE[1] = usCTR_BRIGHTNESS;
                  psBRIGHTNESS_STORAGE[2] = usCTR_BRIGHTNESS;
                  __HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_1,usCTR_BRIGHTNESS);
                  __HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_2,usCTR_BRIGHTNESS);
                  __HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_4,usCTR_BRIGHTNESS);
                }
                else if(pcTEMP_RX[23] == 'L' && pcTEMP_RX[24] == '1')
                {
                  //SET LIGHT L1
                  //printf("L1:%d\r\n",usCTR_BRIGHTNESS);
                  psBRIGHTNESS_STORAGE[0] = usCTR_BRIGHTNESS;
                  __HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_1,usCTR_BRIGHTNESS);
                }
                else if(pcTEMP_RX[23] == 'L' && pcTEMP_RX[24] == '2')
                {
                  //SET LIGHT L2
                  //printf("L2:%d\r\n",usCTR_BRIGHTNESS);
                  psBRIGHTNESS_STORAGE[1] = usCTR_BRIGHTNESS;
                  __HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_2,usCTR_BRIGHTNESS);
                }
                else if(pcTEMP_RX[23] == 'L' && pcTEMP_RX[24] == '3')
                {
                  //SET LIGHT L3
                  //printf("L3:%d\r\n",usCTR_BRIGHTNESS);
                  psBRIGHTNESS_STORAGE[2] = usCTR_BRIGHTNESS;
                  __HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_4,usCTR_BRIGHTNESS);
                }
              }
            }
          }
          printf("AT+MQTTPUB=0,\"$oc/devices/%s/sys/commands/response/request_id=%s\",\"\",0,0\r\n",HUAWEI_MQTT_DeviceID,pcREQUEST_ID);
        }
        memset(SUB_RX_BUF, 0, sizeof(SUB_RX_BUF));
      }
      

			
			for(int i=0;i<UART1_RX_CNT;i++)
				UART1_RX_BUF[i] = 0;
			UART1_RX_CNT = 0;
    }
  }
		HAL_UART_Receive_IT(&huart1, (uint8_t *)UART1_RX_TEMP, 1);
	
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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

#ifdef  USE_FULL_ASSERT
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
