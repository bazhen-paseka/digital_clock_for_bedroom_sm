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

//	uint8_t Alarm_flag = 0 ;
//	uint8_t AlarmHour = 0 ;
//	uint8_t AlarmMin  = 0 ;
	uint8_t night_mode_flag = 0 ;
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
	sprintf(DataChar,"\r\n\r\n\tDigital clock for bedroom v%d.%d.%d %02d/%02d/%d" ,
			soft_version_arr_int[0] , soft_version_arr_int[1] , soft_version_arr_int[2] ,
			version_day_i16 , version_month_i16 , version_year_i16 ) ;
	HAL_UART_Transmit( &huart1, (uint8_t *)DataChar , strlen(DataChar) , 100 ) ;

	#define DATE_as_int_str 	(__DATE__)
	#define TIME_as_int_str 	(__TIME__)
	sprintf(DataChar,"\r\n\tBuild: %s. Time: %s." , DATE_as_int_str , TIME_as_int_str ) ;
	HAL_UART_Transmit( &huart1, (uint8_t *)DataChar , strlen(DataChar) , 100 ) ;

	sprintf(DataChar,"\r\n\tFor debug: UART1-115200/8-N-1" ) ;
	HAL_UART_Transmit( &huart1, (uint8_t *)DataChar , strlen(DataChar) , 100 ) ;


	I2Cdev_init( &hi2c1 ) ;
	I2C_ScanBusFlow( &hi2c1 , &huart1 ) ;

	DS3231_TimeTypeDef TimeSt ;
	DS3231_DateTypeDef DateSt ;
	//	Set_Date_and_Time_to_DS3231( 2021, 3, 8, 4, 3, 52, 40 ) ;
	ds3231_GetTime( ADR_I2C_DS3231 , &TimeSt ) ;
	ds3231_GetDate( ADR_I2C_DS3231 , &DateSt ) ;

	ds3231_PrintDate( 		&DateSt , &huart1 ) ;
	ds3231_PrintWeek_3Char(	&DateSt , &huart1 ) ;
	ds3231_PrintTime( 		&TimeSt , &huart1 ) ;
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
	char DataChar[100];
	DS3231_TimeTypeDef		TimeSt ;
	DS3231_DateTypeDef	 	DateSt ;

	if ( button_u8 > 0 ) {

		_beeper( BEEPER_DELAY ) ;
		ds3231_GetTime( ADR_I2C_DS3231, &TimeSt ) ;
		ds3231_GetDate( ADR_I2C_DS3231, &DateSt ) ;

		if ( button_u8 == 1 ) {
			if ( TimeSt.Hours >= 24 ) {
				TimeSt.Hours = 0 ;
			} else {
				TimeSt.Hours++ ;
			}
			ds3231_SetTime( ADR_I2C_DS3231, &TimeSt ) ;
			max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes ) ;
		}

		if ( button_u8 == 2 ) {
			if ( TimeSt.Hours == 0 ) {
				TimeSt.Hours = 23 ;
			} else {
				TimeSt.Hours-- ;
			}
			ds3231_SetTime( ADR_I2C_DS3231, &TimeSt ) ;
			max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes ) ;
		}

		if ( button_u8 == 3 ) {
			night_mode_flag = 1 ;
			max7219_show_time( &h1_max7219, START_NIGHT_MODE_HOUR, FINISH_NIGHT_MODE_HOUR ) ;
			sprintf( DataChar , "\r\n Night mode - On\r\n" ) ;
			HAL_UART_Transmit( &huart1 , (uint8_t *)DataChar , strlen(DataChar) , 100 ) ;
			//	HAL_IWDG_Refresh( &hiwdg ) ;
		}

//			if ( Alarm_flag == 1 ) {
//				AlarmHour = AlarmHour + ALARM_PERIOD_HOUR ;
//				if ( AlarmHour >= 24 ) {
//					AlarmHour = AlarmHour - 24 ;
//				}
//				AlarmMin  = AlarmMin + ALARM_PERIOD_MIN ;
//				if ( AlarmMin >= 60 ) {
//					AlarmMin = AlarmMin - 60 ;
//					AlarmHour++ ;
//					if ( AlarmHour >= 24 ) {
//						AlarmHour = AlarmHour - 24 ;
//					}
//				}
//			}

//			if ( Alarm_flag == 0 ) {
//				Alarm_flag = 1 ;
//
//				AlarmHour = TimeSt.Hours + ALARM_PERIOD_HOUR ;
//				if ( AlarmHour >= 24 ) {
//					AlarmHour = AlarmHour - 24 ;
//				}
//
//				AlarmMin  = TimeSt.Minutes + ALARM_PERIOD_MIN ;
//				if ( AlarmMin >= 60 ) {
//					AlarmMin = AlarmMin - 60 ;
//					AlarmHour++ ;
//					if ( AlarmHour >= 24 ) {
//						AlarmHour = AlarmHour - 24 ;
//					}
//				}
//			}

//			max7219_show_time( &h1_max7219 , AlarmHour , AlarmMin ) ;
//			sprintf( DataChar , "\r\nAlarm Set %02d:%02d:00\r\n",  AlarmHour , AlarmMin ) ;
//			HAL_UART_Transmit( &huart1 , (uint8_t *)DataChar , strlen(DataChar) , 100 ) ;
//			HAL_Delay( 1000 ) ;

		if ( button_u8 == 4 ) {
			//Alarm_flag = 0 ;
			night_mode_flag = 0 ;
			max7219_show_time( &h1_max7219 , 00 , 00 ) ;
			sprintf( DataChar , "\r\n Night mode - Off\r\n" ) ;
			HAL_UART_Transmit( &huart1 , (uint8_t *)DataChar , strlen(DataChar) , 100 ) ;
		}

		if ( button_u8 == 5 ) {
			if ( TimeSt.Minutes <=0 ) {
				TimeSt.Minutes = 59 ;
			} else {
				TimeSt.Minutes--;
			}
			ds3231_SetTime( ADR_I2C_DS3231, &TimeSt ) ;
			max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes ) ;
		}

		if ( button_u8 == 6 ) {
			if ( TimeSt.Minutes >= 60 ) {
				TimeSt.Minutes = 0;
			} else {
				TimeSt.Minutes++ ;
			}
			ds3231_SetTime( ADR_I2C_DS3231, &TimeSt ) ;
			max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes ) ;
		}

		HAL_Delay( BUTTON_DELAY ) ;
		button_u8 = 0 ;
		HAL_IWDG_Refresh( &hiwdg ) ;
	}

	if ( Ds3231_hard_alarm_flag_Status() == 1 ) {

//		if ((	TimeSt.Hours	== AlarmHour)
//			&&( TimeSt.Minutes	== AlarmMin	)
//			&&( Alarm_flag		== 1		)) {
//			_beeper( BEEPER_DELAY ) ;
//			HAL_Delay( BUTTON_DELAY ) ;
//			_beeper( BEEPER_DELAY ) ;
//		}

		char DataChar[20] ;
		sprintf(DataChar,"\r") ;
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		uint32_t light_u32 = ADC1_GetValue(&hadc1, 1) 	;
		max7219_LED_Intensity	intensity_u8 = Intensity_1 ;
		if ( light_u32 < LIGHT_LEVEL_0)															intensity_u8 = Intensity_1;
		if ((light_u32 > (LIGHT_LEVEL_0 + LIGHT_LEVEL_OFFSET)) && (light_u32 < LIGHT_LEVEL_1))	intensity_u8 = Intensity_5;
		if ((light_u32 > (LIGHT_LEVEL_1 + LIGHT_LEVEL_OFFSET)) && (light_u32 < LIGHT_LEVEL_2))	intensity_u8 = Intensity_9;
		if ((light_u32 > (LIGHT_LEVEL_2 + LIGHT_LEVEL_OFFSET)))									intensity_u8 = Intensity_15;

		sprintf(DataChar,"L=%04i; I=%02d;  ", (int)light_u32 , (int)intensity_u8 ) ;
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		ds3231_GetTime( ADR_I2C_DS3231, &TimeSt ) ;
		ds3231_GetDate( ADR_I2C_DS3231, &DateSt ) ;
		ds3231_PrintDate_AllChar(	&DateSt, &huart1 ) ;
		ds3231_PrintWeek_AllChar(	&DateSt, &huart1 ) ;
		ds3231_PrintTime( 			&TimeSt, &huart1 ) ;

		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin ) ;
		//max7219_init(&h1_max7219, DECODE_MODE, INTENSITY, DISPLAY_DIGIT, WORK_MODE ) ;

//		if (   ( TimeSt.Hours > START_NIGHT_MODE_HOUR  )
//			&& ( TimeSt.Hours < FINISH_NIGHT_MODE_HOUR )) {
//			max7219_init(&h1_max7219, DECODE_MODE, intensity_u8, DISPLAY_DIGIT, WORK_MODE ) ;
//			max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes ) ;
//		} else if 	( night_mode_flag == 1) {
//			max7219_init(&h1_max7219, DECODE_MODE, intensity_u8, DISPLAY_DIGIT, OFF_MODE ) ;
//			//max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes ) ;
//		}

		if 	(  ( night_mode_flag == 1)
			&& (( TimeSt.Hours > START_NIGHT_MODE_HOUR  )
			|| ( TimeSt.Hours < FINISH_NIGHT_MODE_HOUR ))) {
			max7219_init(&h1_max7219, DECODE_MODE, intensity_u8, DISPLAY_DIGIT, OFF_MODE ) ;
		} else {
			max7219_init(&h1_max7219, DECODE_MODE, intensity_u8, DISPLAY_DIGIT, WORK_MODE ) ;
			max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes ) ;
		}

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
