/**
* \file
* \version 1.0
* \author bazhen.levkovets
** \date 2018
*
*************************************************************************************
* \copyright	Bazhen Levkovets
* \copyright	Brovary, Kyiv region
* \copyright	Ukraine
*
*************************************************************************************
*
* \brief
*
*/

/*
**************************************************************************
*							INCLUDE FILES
**************************************************************************
*/

	#include "digital_clock_for_bedroom_sm.h"

/*
**************************************************************************
*							LOCAL DEFINES
**************************************************************************
*/


/*
**************************************************************************
*							LOCAL CONSTANTS
**************************************************************************
*/


/*
**************************************************************************
*						    LOCAL DATA TYPES
**************************************************************************
*/

		max7219_struct h1_max7219 =
		{
			.spi		= &hspi1,
			.cs_port	= SPI1_CS_GPIO_Port,
			.cs_pin		= SPI1_CS_Pin
		};

/*
**************************************************************************
*							  LOCAL TABLES
**************************************************************************
*/

/*
**************************************************************************
*								 MACRO'S
**************************************************************************
*/


/*
**************************************************************************
*						    GLOBAL VARIABLES
**************************************************************************
*/
	volatile uint8_t ds3231_alarm_u8 			= 0 ;

/*
**************************************************************************
*                        LOCAL FUNCTION PROTOTYPES
**************************************************************************
*/

/*
**************************************************************************
*                           GLOBAL FUNCTIONS
**************************************************************************
*/

void Digit_clock_Init (void){

	int soft_version_arr_int[3];
	soft_version_arr_int[0] = ((SOFT_VERSION) / 100) %10 ;
	soft_version_arr_int[1] = ((SOFT_VERSION) /  10) %10 ;
	soft_version_arr_int[2] = ((SOFT_VERSION)      ) %10 ;

	char DataChar[100];
	sprintf(DataChar,"\r\n\r\n\tDigital clock for bedroom v%d.%d.%d 2021-MAR-06\r\n\tFor debug: UART1-115200/8-N-1",
			soft_version_arr_int[0], soft_version_arr_int[1], soft_version_arr_int[2]);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	I2Cdev_init(&hi2c1);
	I2C_ScanBusFlow(&hi2c1, &huart1);

	RTC_TimeTypeDef TimeSt;
	RTC_DateTypeDef DateSt;
	ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
	ds3231_GetDate(ADR_I2C_DS3231, &DateSt);
	//if ( (TimeSt.Hours == 0) && (TimeSt.Minutes == 0) && (TimeSt.Seconds == 0) ) {
	if (TimeSt.Hours == 0) {
		Set_Day_and_Time_to_DS3231 (2021, 03, 06, 17, 41, 01) ;
	}

	ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
	ds3231_GetDate(ADR_I2C_DS3231, &DateSt);
	HAL_RTC_SetTime( &hrtc, &TimeSt, RTC_FORMAT_BIN );
	HAL_RTC_SetDate( &hrtc, &DateSt, RTC_FORMAT_BIN );

	ds3231_PrintDate(&DateSt, &huart1);
	ds3231_PrintTime(&TimeSt, &huart1);
	sprintf(DataChar,"\r\n") ;
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	//ds3231_Alarm1_SetSeconds(ADR_I2C_DS3231, 0x00);
	ds3231_Alarm1_SetEverySeconds(ADR_I2C_DS3231);
	ds3231_Alarm1_ClearStatusBit(ADR_I2C_DS3231);


	max7219_test_LED(&h1_max7219, 300);
	HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
	max7219_init(&h1_max7219, NoDecode, Intensity_5, DisplayDigit_0_7, NormalOperation);
	max7219_show_time(&h1_max7219, 100 + soft_version_arr_int[0] , (soft_version_arr_int[1]*10 + soft_version_arr_int[2]) );
	HAL_Delay(400);

	HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
	max7219_init(&h1_max7219, NoDecode, Intensity_5, DisplayDigit_0_7, NormalOperation);
	max7219_show_time(&h1_max7219, TimeSt.Hours, TimeSt.Minutes);
}
//***************************************************************************

void Digit_clock_Main (void) {
	if (ds3231_alarm_u8 == 1) {

		RTC_TimeTypeDef TimeSt;
		RTC_DateTypeDef DateSt;
		ds3231_GetTime( ADR_I2C_DS3231, &TimeSt ) ;
		ds3231_GetDate( ADR_I2C_DS3231, &DateSt ) ;

		ds3231_PrintDate( &DateSt, &huart1 ) ;
		ds3231_PrintTime( &TimeSt, &huart1 ) ;

		char DataChar[10];
		sprintf(DataChar,"\r") ;
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin ) ;
		max7219_init(&h1_max7219, NoDecode, Intensity_1, DisplayDigit_0_7, NormalOperation ) ;
		max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes ) ;

		ds3231_Alarm1_ClearStatusBit( ADR_I2C_DS3231 ) ;
		ds3231_alarm_u8 = 0 ;
	}
}
//***************************************************************************

void Set_ds3231_alarm (void) {
	ds3231_alarm_u8 = 1;
}
//***************************************************************************


//***************************************************************************

/*
**************************************************************************
*                           LOCAL FUNCTIONS
**************************************************************************
*/
