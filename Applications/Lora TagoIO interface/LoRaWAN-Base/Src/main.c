
/*******************************************************
* File Name        : main.c
* Author           : Henrique Kuhn
* Date             : 19-November-2021
* Description      : Lora -> TagoIO web platform comunication
*********************************************************/

//DEBUG CONFIG FILE:
#ifdef DEBUG
#include "debug_configs.h"
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "uart.h"
#include "spi.h"
#include "rtc.h"
#include "sx126x.h"
#include "sx126x_board.h"
#include "radio.h"
#include "peripheral_init.h"
#include "lorawan_setup.h"
#include "lora-test.h"
#include "LoRaMac.h"
#include "hal_wrappers.h"
#include "i2c.h"
#include "ht_sensors.h"

/*
LoRaWAN related configs
#include "lorawandefines.h"
*/

/**
  * @brief  The application entry point.
  * @retval int
  */ 
int main(void)
{
	
  /* System initialization function */
  if (SystemInit(SYSCLK_64M, BLE_SYSCLK_NONE) != SUCCESS) {
    /* Error during system clock configuration take appropriate action */
    while(1);
  }

	 
	HAL_Init();							/*HAL library hardware initialization*/
	IRQHandler_Config();		/*Interuption request initialization*/	
	MX_GPIO_Init();					/*GPIO initialization*/	
	MX_USART1_UART_Init();	/*USART Initialization*/	
	MX_SPI1_Init();					/*SPI Initialization*/	
	MX_RTC_Init();					/*RTC Initialization*/
	MX_I2C2_Init();					/*Initializes LoRaWAN stack and radio*/
			
	LORAWAN_init(DEFAULT_REGION);	/*initialize LoRaWan radio parameters*/
	
	initSensors();		/*Initialize sensors config.*/
	
	while (1){
			
		LORAWAN_tick(); /*handle radio routine*/

  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
	printf("error\n");
  while(1);
}



#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
F
  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
