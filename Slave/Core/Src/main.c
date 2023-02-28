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
#include "dma.h"
#include "fatfs.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SD_FATFS.h"
#include <stdio.h>
#include "mcp3208.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIMEDEBUG
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//Sampling   Ts=55us
//Ts->3 byte
//T1byte=Ts/3
//2040*55/3=37400
//180000 *3/2040
//Ts=264

//while loop



volatile uint8_t ACV=0;
uint8_t  convnumber=0;

//DMA
extern uint8_t databuf[BUFLEN];
extern SPI_HandleTypeDef hspi4;
extern DMA_HandleTypeDef hdma_spi4_rx;
extern DMA_HandleTypeDef hdma_tim1_ch1;
volatile uint8_t firstconversion=0;
volatile uint8_t secondconversion = 0;
volatile uint16_t adcdata[CHAN];
//SD

extern struct SD_Iterface SD;

char myfilename[]="DATA.txt";
uint8_t savestart=1;
uint8_t savestop=0;
//time Debug
volatile uint32_t thalfread1,thalfread2,dmaArraytime;
uint32_t tsaveSD1,tsaveSD2;
uint32_t tconver1,tconver2;





/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void XferHCplt(DMA_HandleTypeDef *hdma);
void Farray(DMA_HandleTypeDef *hdma);
void Sarray(DMA_HandleTypeDef *hdma);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void conversion(uint8_t Whalf){

//16320
	if(Whalf==1){
		for( uint16_t i=0;i<BUFLEN/12;i++){
			SD.buffer1[3*i]='s';
			SD.buffer1[3*i+1]=databuf[3*i+1];
			SD.buffer1[3*i+2]=databuf[3*i+2];
		}
		for(uint16_t i=0;i<BUFLEN/12;i++){

			SD.buffer2[3*i]='s';
			SD.buffer2[3*i+1]=databuf[3*i+1+BUFLEN/4];
			SD.buffer2[3*i+2]=databuf[3*i+2+BUFLEN/4];


		}
	}
	else{
		for( uint16_t i=0;i<BUFLEN/12;i++){
			SD.buffer3[3*i]='s';
			SD.buffer3[3*i+1]=databuf[3*i+1+BUFLEN/2];
			SD.buffer3[3*i+2]=databuf[3*i+2+BUFLEN/2];

		}
		for( uint16_t i=0;i<BUFLEN/12;i++){
			SD.buffer4[3*i]='s';
			SD.buffer4[3*i+1]=databuf[3*i+1+BUFLEN/2+BUFLEN/4];
			SD.buffer4[3*i+2]=databuf[3*i+2+BUFLEN/2+BUFLEN/4];

		}


	}


}

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
  MX_SPI4_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  mountSDcard();
  HAL_TIM_Base_Start(&htim2);
  hdma_tim1_ch1.XferHalfCpltCallback=XferHCplt;

  InitMCP3208(hspi4, htim1, TIM_CHANNEL_1,htim3, TIM_CHANNEL_1, TIM_CHANNEL_2);
  hdma_spi4_rx.XferHalfCpltCallback=Farray;
    hdma_spi4_rx.XferCpltCallback=Sarray;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  if(firstconversion){
		  conversion( 1);
		  firstconversion=0;
	  }
	  if(secondconversion){

		  conversion( 2);

#ifdef TIMEDEBUG
		  tsaveSD1=TIM2->CNT;
#endif
		  if(ACV==1){
			  if(savestart){
				  openFile(myfilename, FA_OPEN_APPEND | FA_WRITE | FA_READ);
				  savestart=0;
				  savestop=1;
			  }
			  writeDataPacked(myfilename, FA_OPEN_APPEND | FA_WRITE | FA_READ);
			  HAL_UART_Transmit(&huart2,(uint8_t *)SD.buffer1 , BUFLEN/4, 10);
			  HAL_UART_Transmit(&huart2,(uint8_t *)SD.buffer2 , BUFLEN/4, 10);
			  HAL_UART_Transmit(&huart2,(uint8_t *)SD.buffer3 , BUFLEN/4, 10);
			  HAL_UART_Transmit(&huart2,(uint8_t *)SD.buffer4, BUFLEN/4, 10);
		    }
		  if(ACV==0 && savestop==1){
			  savestart=1;
			  savestop=0;
			  closeFile();
		  }
		  if(SD.fresult!=FR_OK){
			  HAL_GPIO_WritePin(SD_RST_GPIO_Port,SD_RST_Pin,GPIO_PIN_SET);

		  }

#ifdef TIMEDEBUG
		  tsaveSD2=TIM2->CNT-tsaveSD1;
#endif



	  	secondconversion=0;

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void XferHCplt(DMA_HandleTypeDef *hdma) {
	 adcdata[0]=databuf[2] | (((uint16_t) (databuf[1] & 0x0f)) << 8);
	 adcdata[1]=databuf[5] | (((uint16_t) (databuf[4] & 0x0f)) << 8);
	 adcdata[2]=databuf[8] | (((uint16_t) (databuf[7] & 0x0f)) << 8);
	 adcdata[3]=databuf[11] | (((uint16_t) (databuf[10] & 0x0f)) << 8);
	 adcdata[4]=databuf[14] | (((uint16_t) (databuf[13] & 0x0f)) << 8);
	 adcdata[5]=databuf[17] | (((uint16_t) (databuf[16] & 0x0f)) << 8);
	 adcdata[6]=databuf[20] | (((uint16_t) (databuf[19] & 0x0f)) << 8);
	 adcdata[7]=databuf[23] | (((uint16_t) (databuf[22] & 0x0f)) << 8);

}


void Farray(DMA_HandleTypeDef *hdma) {
	firstconversion=1;
#ifdef TIMEDEBUG
	thalfread1=TIM2->CNT;
#endif
}

void Sarray(DMA_HandleTypeDef *hdma) {
	secondconversion = 1;
#ifdef TIMEDEBUG
	thalfread2=TIM2->CNT-thalfread1;
	dmaArraytime=thalfread2*2;
#endif
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if(GPIO_Pin==ACV_Pin){
		if(HAL_GPIO_ReadPin(ACV_GPIO_Port, ACV_Pin)){
			ACV=1;
		}
		else{
			ACV=0;
		}

	}


}

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
