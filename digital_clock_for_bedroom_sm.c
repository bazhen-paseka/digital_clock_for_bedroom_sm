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
	max7219_struct 	h1_max7219 ;
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

	uint8_t button_u8 = 0 ;
/*
**************************************************************************
*                        LOCAL FUNCTION PROTOTYPES
**************************************************************************
*/
	void _beeper( uint32_t _time_u32 ) ;
/*
**************************************************************************
*                           GLOBAL FUNCTIONS
**************************************************************************
*/

void Digit_clock_Init (void) {
	_beeper( 100 ) ;
	int soft_version_arr_int[3];
	soft_version_arr_int[0] = ((SOFT_VERSION) / 100) %10 ;
	soft_version_arr_int[1] = ((SOFT_VERSION) /  10) %10 ;
	soft_version_arr_int[2] = ((SOFT_VERSION)      ) %10 ;

	int16_t version_year_i16	= VERSION_YEAR	;
	int16_t version_month_i16 	= VERSION_MONTH	;
	int16_t version_day_i16		= VERSION_DAY	;

	char DataChar[100];
	sprintf(DataChar,"\r\n\r\n\tDigital clock for bedroom v%d.%d.%d %02d/%02d/%d\r\n\tFor debug: UART1-115200/8-N-1" ,
			soft_version_arr_int[0] , soft_version_arr_int[1] , soft_version_arr_int[2] ,
			version_day_i16 , version_month_i16 , version_year_i16 ) ;
	HAL_UART_Transmit( &huart1, (uint8_t *)DataChar , strlen(DataChar) , 100 ) ;

	I2Cdev_init( &hi2c1 ) ;
	I2C_ScanBusFlow( &hi2c1 , &huart1 ) ;

	RTC_TimeTypeDef TimeSt ;
	RTC_DateTypeDef DateSt ;
	ds3231_GetTime( ADR_I2C_DS3231 , &TimeSt ) ;
	ds3231_GetDate( ADR_I2C_DS3231 , &DateSt ) ;
//	if ( TimeSt.Hours == 10 ) {
//		Set_Day_and_Time_to_DS3231( 2021, 03, 12, 00, 45, 45 ) ;
//	}

	ds3231_GetTime( ADR_I2C_DS3231 , &TimeSt ) ;
	ds3231_GetDate( ADR_I2C_DS3231 , &DateSt ) ;
	HAL_RTC_SetTime( &hrtc , &TimeSt , RTC_FORMAT_BIN ) ;
	HAL_RTC_SetDate( &hrtc , &DateSt , RTC_FORMAT_BIN );

	ds3231_PrintDate( &DateSt , &huart1 ) ;
	ds3231_PrintTime( &TimeSt , &huart1 ) ;
	sprintf( DataChar , "\r\n" ) ;
	HAL_UART_Transmit( &huart1 , (uint8_t *)DataChar , strlen(DataChar) , 100 ) ;

	//ds3231_Alarm1_SetSeconds(ADR_I2C_DS3231, 0x00) ;
	ds3231_Alarm1_SetEverySeconds( ADR_I2C_DS3231 ) ;
	ds3231_Alarm1_ClearStatusBit ( ADR_I2C_DS3231 ) ;

	Max7219_struct_init (&h1_max7219, &hspi1,SPI1_CS_GPIO_Port,SPI1_CS_Pin);
	//	max7219_test_LED( &h1_max7219 , 300 ) ;
	max7219_init(&h1_max7219, DECODE_MODE, INTENSITY, DISPLAY_DIGIT, WORK_MODE);
	max7219_show_time( &h1_max7219 , 100 + soft_version_arr_int[0] , (soft_version_arr_int[1]*10 + soft_version_arr_int[2]) ) ;

	HAL_GPIO_TogglePin( LED_RED_GPIO_Port , LED_RED_Pin ) ;
	HAL_IWDG_Refresh( &hiwdg ) ;
}
//***************************************************************************

void Digit_clock_Main (void) {

	RTC_TimeTypeDef		TimeSt ;
	RTC_DateTypeDef 	DateSt ;

	if ( button_u8 == 1 ) {
		_beeper( BEEPER_DELAY ) ;
		ds3231_GetTime( ADR_I2C_DS3231, &TimeSt ) ;
		ds3231_GetDate( ADR_I2C_DS3231, &DateSt ) ;
		TimeSt.Hours++ ;
		if ( TimeSt.Hours >= 24 ) TimeSt.Hours = 0 ;
		Set_Date_and_Time_by_str( &DateSt, &TimeSt ) ;
		max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes ) ;
		HAL_Delay( BUTTON_DELAY ) ;
		button_u8 = 0 ;
		HAL_IWDG_Refresh( &hiwdg ) ;
	}

	if ( button_u8 == 2 ) {
		_beeper( BEEPER_DELAY ) ;
		ds3231_GetTime( ADR_I2C_DS3231, &TimeSt ) ;
		ds3231_GetDate( ADR_I2C_DS3231, &DateSt ) ;
		if ( TimeSt.Hours == 0 ) {
			TimeSt.Hours = 23 ;
		} else {
			TimeSt.Hours-- ;
		}
		Set_Date_and_Time_by_str( &DateSt, &TimeSt ) ;
		max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes ) ;
		HAL_Delay( BUTTON_DELAY ) ;
		button_u8 = 0 ;
		HAL_IWDG_Refresh( &hiwdg ) ;
	}

	if ( button_u8 == 3 ) {
		_beeper( BEEPER_DELAY ) ;
		ds3231_GetTime( ADR_I2C_DS3231, &TimeSt ) ;
		ds3231_GetDate( ADR_I2C_DS3231, &DateSt ) ;
		max7219_show_time( &h1_max7219 , 99 , 99 ) ;
		HAL_Delay( BUTTON_DELAY ) ;
		button_u8 = 0 ;
		HAL_IWDG_Refresh( &hiwdg ) ;
	}

	if ( button_u8 == 4 ) {
		_beeper( BEEPER_DELAY ) ;

		ds3231_GetTime( ADR_I2C_DS3231, &TimeSt ) ;
		ds3231_GetDate( ADR_I2C_DS3231, &DateSt ) ;
		max7219_show_time( &h1_max7219 , 00 , 00 ) ;
		HAL_Delay( BUTTON_DELAY ) ;
		button_u8 = 0 ;
		HAL_IWDG_Refresh( &hiwdg ) ;
	}

	if ( button_u8 == 5 ) {
		if ( HAL_GPIO_ReadPin( BUTTON_6_GPIO_Port, BUTTON_6_Pin ) == GPIO_PIN_RESET ) {
			_beeper( BEEPER_DELAY ) ;
			ds3231_GetTime( ADR_I2C_DS3231, &TimeSt ) ;
			ds3231_GetDate( ADR_I2C_DS3231, &DateSt ) ;
			TimeSt.Minutes++ ;
			if ( TimeSt.Minutes >=60 ) TimeSt.Minutes = 0;
			Set_Date_and_Time_by_str( &DateSt, &TimeSt ) ;
			max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes ) ;
			HAL_Delay( BUTTON_DELAY ) ;
			button_u8 = 0 ;
			HAL_IWDG_Refresh( &hiwdg ) ;
		}

		if ( HAL_GPIO_ReadPin( BUTTON_5_GPIO_Port, BUTTON_5_Pin ) == GPIO_PIN_RESET ) {
			_beeper( BEEPER_DELAY ) ;

			ds3231_GetTime( ADR_I2C_DS3231, &TimeSt ) ;
			ds3231_GetDate( ADR_I2C_DS3231, &DateSt ) ;
			if ( TimeSt.Minutes <=0 ) {
				TimeSt.Minutes = 59 ;
			} else {
			TimeSt.Minutes--;
			}
			Set_Date_and_Time_by_str( &DateSt, &TimeSt ) ;
			max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes ) ;
			HAL_Delay( BUTTON_DELAY ) ;
			button_u8 = 0 ;
			HAL_IWDG_Refresh( &hiwdg ) ;
		}

	}


	if ( Ds3231_hard_alarm_flag_Status() == 1 ) {

		ds3231_GetTime( ADR_I2C_DS3231, &TimeSt ) ;
		ds3231_GetDate( ADR_I2C_DS3231, &DateSt ) ;
		ds3231_PrintDate( &DateSt, &huart1 ) ;
		ds3231_PrintTime( &TimeSt, &huart1 ) ;
		char DataChar[10] ;
		sprintf(DataChar,"\r") ;
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin ) ;
		max7219_init(&h1_max7219, DECODE_MODE, INTENSITY, DISPLAY_DIGIT, WORK_MODE ) ;
		max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes ) ;

		ds3231_Alarm1_ClearStatusBit( ADR_I2C_DS3231 ) ;
		Ds3231_hard_alarm_flag_Reset() ;
		HAL_IWDG_Refresh( &hiwdg ) ;
	}
}

/*
**************************************************************************
*                           LOCAL FUNCTIONS
**************************************************************************
*/

void _beeper( uint32_t _time_u32 ) {
	HAL_GPIO_WritePin( BEEPER_1_GPIO_Port , BEEPER_1_Pin , RESET ) ;
	HAL_Delay( _time_u32 ) ;
	HAL_GPIO_WritePin( BEEPER_1_GPIO_Port , BEEPER_1_Pin , SET ) ;
}
/***************************************************************************************/

void Digit_clock_Set_button(uint8_t _button) {
	button_u8 = _button ;
}
/*
**************************************************************************
*                                   END
**************************************************************************
*/
