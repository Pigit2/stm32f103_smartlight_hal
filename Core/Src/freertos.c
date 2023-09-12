/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "tim.h"
#include "adc.h"
#include "rtc.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
RTC_DateTypeDef sDATE;                 // HAL_RTC_GetDate STRUCTURE
RTC_TimeTypeDef sTIME;                 // HAL_RTC_GetTime STRUCTURE
uint8_t usLOWER_HALF_FLAG_SAVE = 0;    // PREVIOUS LOWER FLAG SAVING USE FOR RESUME PREVIOUS FLAG
uint16_t psBRIGHTNESS_SAVE[3] = {0};   // WHEN ADJUST BRIGHTNESS AND TIMING OFF, SAVE PREVIOUS BRIGHTNESS
extern char pcWEEK[5];
extern uint8_t usLOWER_HALF_SFLAG;
extern uint16_t usTIMING_HOUR, usTIMING_MIN, usTIMING_SEC;
extern uint8_t usTIMING_CMD;
extern uint16_t psBRIGHTNESS_STORAGE[3];
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId MAIN_TaskHandle;
osThreadId KEY_PAGE_TaskHandle;
osTimerId JitterTimer01Handle;
osTimerId JitterTimer02Handle;
osTimerId GET_ADCTimerHandle;
osSemaphoreId myBinarySem01Handle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void Start_MAIN_Task(void const * argument);
void Start_KEY_PAGE_Task(void const * argument);
void JitterTimerCallback01(void const * argument);
void JitterTimerCallback02(void const * argument);
void GET_ADCTimerCallback(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of myBinarySem01 */
  osSemaphoreDef(myBinarySem01);
  myBinarySem01Handle = osSemaphoreCreate(osSemaphore(myBinarySem01), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of JitterTimer01 */
  osTimerDef(JitterTimer01, JitterTimerCallback01);
  JitterTimer01Handle = osTimerCreate(osTimer(JitterTimer01), osTimerOnce, NULL);

  /* definition and creation of JitterTimer02 */
  osTimerDef(JitterTimer02, JitterTimerCallback02);
  JitterTimer02Handle = osTimerCreate(osTimer(JitterTimer02), osTimerOnce, NULL);

  /* definition and creation of GET_ADCTimer */
  osTimerDef(GET_ADCTimer, GET_ADCTimerCallback);
  GET_ADCTimerHandle = osTimerCreate(osTimer(GET_ADCTimer), osTimerPeriodic, NULL);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */

  osTimerStart(JitterTimer01Handle, 200);   // START TIMER1 ONCE AND SET TIME(ms)
	osTimerStop(JitterTimer01Handle);         // STOP TIMER1, SET TIME NEED START
  osTimerStart(JitterTimer02Handle, 200);   // START TIMER2 ONCE AND SET TIME(ms)
  osTimerStop(JitterTimer02Handle);         // STOP TIMER2, SET TIME NEED START
  osTimerStart(GET_ADCTimerHandle, 5000);   // START TIMER3 PERIODIC AND SET TIME(ms)
	
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of MAIN_Task */
  osThreadDef(MAIN_Task, Start_MAIN_Task, osPriorityIdle, 0, 128);
  MAIN_TaskHandle = osThreadCreate(osThread(MAIN_Task), NULL);

  /* definition and creation of KEY_PAGE_Task */
  osThreadDef(KEY_PAGE_Task, Start_KEY_PAGE_Task, osPriorityIdle, 0, 128);
  KEY_PAGE_TaskHandle = osThreadCreate(osThread(KEY_PAGE_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
	for(;;)
  {	
		
		//HAL_GPIO_TogglePin(GPIOC,LED1_Pin);
		
//		__HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_1,val);
//		__HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_2,val);
//		__HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_4,val);
		
//		if(UART1_RX_FLAG)
//    {
//      HAL_UART_Transmit(&huart1, UART1_RX_BUF, UART1_RX_CNT, 0x10);
//      for(int i=0;i<UART1_RX_CNT;i++)
//        UART1_RX_BUF[i] = 0;
//      UART1_RX_CNT = 0;
//      UART1_RX_FLAG = 0;
//    }
		
    osThreadSuspend(defaultTaskHandle);

		/*TASK2 BACKUP*/
		// HAL_RTC_GetTime(&hrtc, &sTIME, RTC_FORMAT_BIN);
		// HAL_RTC_GetDate(&hrtc, &sDATE, RTC_FORMAT_BIN);
		
		// printf("%d/%02d/%02d,%02d:%02d:%02d\r\n",2000+sDATE.Year,sDATE.Month,sDATE.Date,sTIME.Hours,sTIME.Minutes,sTIME.Seconds);
		//ADC_get_voltage();
    //printf("%d/%d/%d,%d:%d:%d\r\n",usYEAR,usMONTH,usDATE,usHOUR,usMIN,usSEC);
		
		//OLED_ShowString(0,0,(u8 *)"hello,test",16);
    //printf("voltage: %f, current: %f, power: %f \r\n",Voltage_value(),Current_value(),Power_value());
    //osDelay(1000);
		

//		if(ledflag==0)
//			val++;
//		if(ledflag==1)
//			val--;
//		if(val==999)
//			ledflag=1;
//		if(val==0)
//			ledflag=0;
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_Start_MAIN_Task */
/**
* @brief Function implementing the MAIN_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_MAIN_Task */
void Start_MAIN_Task(void const * argument)
{
  /* USER CODE BEGIN Start_MAIN_Task */
  /* Infinite loop */
  for(;;)
  {
    char str[9];
    char set_temp[10];

    HAL_GPIO_TogglePin(GPIOC,LED1_Pin);

    HAL_RTC_GetTime(&hrtc, &sTIME, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &sDATE, RTC_FORMAT_BIN);

    str[0] = sTIME.Hours/10+'0';
    str[1] = sTIME.Hours%10+'0';	str[2] = ':';
    str[3] = sTIME.Minutes/10+'0';
    str[4] = sTIME.Minutes%10+'0';str[5] = ':';
    str[6] = sTIME.Seconds/10+'0';
    str[7] = sTIME.Seconds%10+'0';
    str[8] = '\0';
    OLED_ShowString(0,2,(u8 *)str,16);  // DISPLAY TIME

    OLED_ShowNum(0,0,sDATE.Year+2000,4,16);
    OLED_ShowCHinese(32,0,0);    // DISPLAY YEAR
    if(sDATE.Month < 10)
    {
      OLED_ShowNum(48,0,0,1,16);
      OLED_ShowNum(56,0,sDATE.Month,1,16);
      OLED_ShowCHinese(64,0,1);  // DISPLAY MONTH SINGLE
    }
    else
    {
      OLED_ShowNum(48,0,sDATE.Month,2,16);
      OLED_ShowCHinese(64,0,1);  // DISPLAY MONTH
    }
    if(sDATE.Date < 10)
    {
      OLED_ShowNum(80,0,0,1,16);
      OLED_ShowNum(88,0,sDATE.Date,1,16);
      OLED_ShowCHinese(96,0,2);  // DISPLAY DATE SINGLE
    }
    else
    {
      OLED_ShowNum(80,0,sDATE.Date,2,16);
      OLED_ShowCHinese(96,0,2);  // DISPLAY DATE
    }
    OLED_ShowString(80,2,(u8 *)pcWEEK,16);

    if(usLOWER_HALF_SFLAG == 0)  // FLAG FOR POWER
    {
      OLED_ShowCHinese2(16,6,9,3); // SHOW THE POWER
      OLED_Showdecimal(64,6,Power_value(),2,2,16);
      OLED_ShowChar(102,6,'W',16);
    }
    else if (usLOWER_HALF_SFLAG == 1) //SHOW THE TIMING
    {
      set_temp[0] = usTIMING_HOUR/10+'0';
      set_temp[1] = usTIMING_HOUR%10+'0'; set_temp[2] = ':';
      set_temp[3] = usTIMING_MIN/10+'0';
      set_temp[4] = usTIMING_MIN%10+'0';  set_temp[5] = ':';
      set_temp[6] = usTIMING_SEC/10+'0';
      set_temp[7] = usTIMING_SEC%10+'0';
      set_temp[8] = '\0';

      //OLED_Clear();  //DELETE THE OUTDATE POWER
      OLED_ShowString(0,6,(u8 *)"SET ",24);
      OLED_ShowString(32,6,(u8 *)set_temp,24); 
    }
    osDelay(1000);
  }
  /* USER CODE END Start_MAIN_Task */
}

/* USER CODE BEGIN Header_Start_KEY_PAGE_Task */
/**
* @brief Function implementing the KEY_PAGE_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_KEY_PAGE_Task */
void Start_KEY_PAGE_Task(void const * argument)
{
  /* USER CODE BEGIN Start_KEY_PAGE_Task */
  /* Infinite loop */
  for(;;)
  {
    if(usLOWER_HALF_SFLAG == 2) // DISPLAY THE VOLTAGE
    {
      OLED_ShowString(0,6,(u8 *)"              ",16); //CLEAN LOWER HALF
      OLED_ShowCHinese2(16,6,3,3);
      OLED_Showdecimal(64,6,Voltage_value(),2,2,16);
      OLED_ShowChar(102,6,'V',16);
    }
    else if (usLOWER_HALF_SFLAG == 3) // DISPLAY THE CURRENT
    {
      OLED_ShowString(0,6,(u8 *)"              ",16); //CLEAN LOWER HALF
      OLED_ShowCHinese2(16,6,6,3);
      OLED_Showdecimal(64,6,Current_value(),2,2,16);
      OLED_ShowChar(102,6,'A',16);
    }
    osDelay(2000);
    usLOWER_HALF_SFLAG = usLOWER_HALF_FLAG_SAVE;
    usLOWER_HALF_FLAG_SAVE = 0;
    OLED_ShowString(0,6,(u8 *)"              ",16); //CLEAN LOWER HALF
    osThreadSuspend(KEY_PAGE_TaskHandle);
  }
  /* USER CODE END Start_KEY_PAGE_Task */
}

/* JitterTimerCallback01 function */
void JitterTimerCallback01(void const * argument)
{
  /* USER CODE BEGIN JitterTimerCallback01 */
  //printf("<KEY1 PRESS>");
  BaseType_t YieldRequired;
  usLOWER_HALF_FLAG_SAVE = usLOWER_HALF_SFLAG;
  usLOWER_HALF_SFLAG = 2;
  YieldRequired = xTaskResumeFromISR(KEY_PAGE_TaskHandle);
  if(YieldRequired == pdTRUE)
  {
    portYIELD_FROM_ISR(YieldRequired);
  }
  /* USER CODE END JitterTimerCallback01 */
}

/* JitterTimerCallback02 function */
void JitterTimerCallback02(void const * argument)
{
  /* USER CODE BEGIN JitterTimerCallback02 */ 
  //printf("<KEY2 PRESS>");
  BaseType_t YieldRequired;
  usLOWER_HALF_FLAG_SAVE = usLOWER_HALF_SFLAG;
  usLOWER_HALF_SFLAG = 3;
  YieldRequired = xTaskResumeFromISR(KEY_PAGE_TaskHandle);
  if(YieldRequired == pdTRUE)
  {
    portYIELD_FROM_ISR(YieldRequired);
  }
  /* USER CODE END JitterTimerCallback02 */
}

/* GET_ADCTimerCallback function */
void GET_ADCTimerCallback(void const * argument)
{
  /* USER CODE BEGIN GET_ADCTimerCallback */
  ADC_GET_VOLTAGE();
  HuaweiIot_DevDate_publish3((char*)"voltage",Voltage_value(),(char*)"current",Current_value(),(char*)"power",Power_value());
  osDelay(500);
  HuaweiIot_DevDate_publish3((char*)"L1",psBRIGHTNESS_STORAGE[0]/10,(char*)"L2",psBRIGHTNESS_STORAGE[1]/10,(char*)"L3",psBRIGHTNESS_STORAGE[2]/10);
  /* USER CODE END GET_ADCTimerCallback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */



/**
 * @description: GPIO interrupt callback : key to printf.
 * @param {uint16_t} GPIO_Pin
 * @return {*}
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  BaseType_t pxHigherPriorityTaskWoken1 = pdFALSE;
  BaseType_t pxHigherPriorityTaskWoken2 = pdFALSE;
	switch(GPIO_Pin)
	{
		case KEY1_Pin : 
      xTimerResetFromISR(JitterTimer01Handle, &pxHigherPriorityTaskWoken1);
      portYIELD_FROM_ISR(pxHigherPriorityTaskWoken1); //Trigger timer scheduling
      break;
		case KEY2_Pin : 
      xTimerResetFromISR(JitterTimer02Handle, &pxHigherPriorityTaskWoken2);
      portYIELD_FROM_ISR(pxHigherPriorityTaskWoken2);
      break;
		default : break;
	}
}

/**
 * @description: RTC ALARM TIMEOUT CALLBACK FUNCTION
 * @param {RTC_HandleTypeDef} *hrtc
 * @return {*}
 */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  //printf("RTC ALARM INTERRUPT\r\n");
  if(usTIMING_CMD == 1) //TIMING ON
  {
    if (psBRIGHTNESS_STORAGE[0] == 0 && psBRIGHTNESS_STORAGE[1] == 0 && psBRIGHTNESS_STORAGE[2] == 0)
    {
      psBRIGHTNESS_STORAGE[0] = 450;
      psBRIGHTNESS_STORAGE[1] = 450;
      psBRIGHTNESS_STORAGE[2] = 450;
    }
    if(psBRIGHTNESS_SAVE[0] != 0 || psBRIGHTNESS_SAVE[1] != 0 || psBRIGHTNESS_SAVE[2] != 0)
    {
      psBRIGHTNESS_STORAGE[0] = psBRIGHTNESS_SAVE[0];
      psBRIGHTNESS_STORAGE[1] = psBRIGHTNESS_SAVE[1];
      psBRIGHTNESS_STORAGE[2] = psBRIGHTNESS_SAVE[2];
    }
    __HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_1,psBRIGHTNESS_STORAGE[0]);
		__HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_2,psBRIGHTNESS_STORAGE[1]);
		__HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_4,psBRIGHTNESS_STORAGE[2]);
  }
  else                   //TIMING OFF
  {
    psBRIGHTNESS_SAVE[0] = psBRIGHTNESS_STORAGE[0];
    psBRIGHTNESS_SAVE[1] = psBRIGHTNESS_STORAGE[1];
    psBRIGHTNESS_SAVE[2] = psBRIGHTNESS_STORAGE[2];
    psBRIGHTNESS_STORAGE[0] = 0;
    psBRIGHTNESS_STORAGE[1] = 0;
    psBRIGHTNESS_STORAGE[2] = 0;
    __HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_1,psBRIGHTNESS_STORAGE[0]);
    __HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_2,psBRIGHTNESS_STORAGE[1]);
		__HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_4,psBRIGHTNESS_STORAGE[2]);
  }
  usTIMING_CMD = 0;
  usLOWER_HALF_SFLAG = 0;
  OLED_ShowString(0,6,(u8 *)"              ",16); // CLEAN LOWER HALF
}

/* USER CODE END Application */

