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

	#define		LIGHT_CIRCLE_QNT	3
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
	char 	debugChar[100]	= {0};
	uint8_t button_u8 		= 0 ;
	uint8_t night_mode_flag = 1 ;

/*
**************************************************************************
*                        LOCAL FUNCTION PROTOTYPES
**************************************************************************
*/
	void _beeper( uint32_t _time_u32 ) ;
	void Debug	(char* _text);
/*
**************************************************************************
*                           GLOBAL FUNCTIONS
**************************************************************************
*/

void Digit_clock_Init (void) {
	_beeper( 100 ) ;
	int ver[3];
	ver[0] = ((SOFT_VERSION) / 100) %10 ;
	ver[1] = ((SOFT_VERSION) /  10) %10 ;
	ver[2] = ((SOFT_VERSION)      ) %10 ;

	sprintf(debugChar,"\r\n\r\n\tDIEGO - dot clock for bedroom v%d.%d.%d",ver[0], ver[1], ver[2]); Debug(debugChar);
	#define DATE_as_int_str 	(__DATE__)
	#define TIME_as_int_str 	(__TIME__)
	sprintf(debugChar,"\r\n\tBuild: %s. Time: %s." , DATE_as_int_str , TIME_as_int_str); Debug(debugChar);
	sprintf(debugChar,"\r\n\tFor debug: UART1-115200/8-N-1"); Debug(debugChar);

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
	sprintf( debugChar , "\r\n"); Debug(debugChar);

	//ds3231_Alarm1_SetSeconds(ADR_I2C_DS3231, 0x00) ;
	ds3231_Alarm1_SetEverySeconds( ADR_I2C_DS3231 ) ;
	ds3231_Alarm1_ClearStatusBit ( ADR_I2C_DS3231 ) ;

	Max7219_struct_init (&h1_max7219, &hspi1,SPI1_CS_GPIO_Port,SPI1_CS_Pin);
	//	max7219_test_LED( &h1_max7219 , 300 ) ;
	max7219_init(&h1_max7219, DECODE_MODE, INTENSITY_INIT, DISPLAY_DIGIT, WORK_MODE);
	max7219_show_time( &h1_max7219 , 100 + ver[0] , ver[1]*10 + ver[2]  ) ;
	ADC1_Init( &hadc1, ADC_CHANNEL_1);
	HAL_Delay(1000);
	HAL_GPIO_TogglePin( LED_RED_GPIO_Port , LED_RED_Pin ) ;
	HAL_IWDG_Refresh( &hiwdg ) ;
}
//***************************************************************************

void Digit_clock_Main (void) {
	DS3231_TimeTypeDef		TimeSt ;
	DS3231_DateTypeDef	 	DateSt ;

	static max7219_LED_Intensity	intensity_enum			= INTENSITY_MAX ;
	static max7219_LED_Intensity	previous_intensity_enum	= INTENSITY_MAX ;
	static uint32_t 				total_light_u32 		= 0 ;

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
			max7219_show_time( &h1_max7219, START_NIGHT_MODE_HOUR, 00) ;
			//sprintf( debugChar , "\r\n Night mode - On\r\n"); Debug(debugChar);
		}

		if ( button_u8 == 4 ) {
			night_mode_flag = 1 ;
			max7219_show_time( &h1_max7219 , FINISH_NIGHT_MODE_HOUR, 00 ) ;
			//sprintf( debugChar , "\r\n Night mode - Off\r\n"); Debug(debugChar);
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
		sprintf(debugChar,"\r"); Debug(debugChar);

		uint32_t light_u32 = ADC1_GetValue( &hadc1, ADC_CHANNEL_1);
		total_light_u32 = total_light_u32 + light_u32;

		sprintf(debugChar,"Lux=%04lu; Int=%d;  ", light_u32, intensity_enum); Debug(debugChar);

		ds3231_GetTime( ADR_I2C_DS3231, &TimeSt ) ;
		ds3231_GetDate( ADR_I2C_DS3231, &DateSt ) ;
		ds3231_PrintDate_AllChar(	&DateSt, &huart1 ) ;
		ds3231_PrintWeek_AllChar(	&DateSt, &huart1 ) ;
		ds3231_PrintTime( 			&TimeSt, &huart1 ) ;

		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin ) ;

		if ( TimeSt.Seconds % LIGHT_CIRCLE_QNT == 0) {
			uint32_t final_light_u32 = total_light_u32 / LIGHT_CIRCLE_QNT;

			sprintf(debugChar,"\r\ntotal Lux=%lu, final Lux=%lu, ", total_light_u32, final_light_u32); Debug(debugChar);

			total_light_u32 = 0;

			if ( final_light_u32 <= LIGHT_LEVEL_MIN )	intensity_enum = INTENSITY_MIN;
			if ( final_light_u32 >= LIGHT_LEVEL_MAX )	intensity_enum = INTENSITY_MAX;

			if (   ( final_light_u32 > LIGHT_LEVEL_MIN )
				&& ( final_light_u32 < LIGHT_LEVEL_MAX ))	{
				uint32_t int_tmp_u32 = (uint32_t)(INTENSITY_MAX - INTENSITY_MIN);
				uint32_t tmp_u32 = int_tmp_u32 * (final_light_u32 - LIGHT_LEVEL_MIN) / (LIGHT_LEVEL_MAX - LIGHT_LEVEL_MIN) ;
				intensity_enum = (uint8_t)tmp_u32;
			}

			sprintf(debugChar,"current intensity=%d, ", intensity_enum); Debug(debugChar);

			if (     (intensity_enum > previous_intensity_enum)
				&& ( (intensity_enum - previous_intensity_enum) > 1 )) {
				intensity_enum = previous_intensity_enum + 1;
			}

			if (     (intensity_enum < previous_intensity_enum)
				&& ( (previous_intensity_enum - intensity_enum) > 1 )) {
				intensity_enum = previous_intensity_enum - 1;
			}

			sprintf(debugChar,"next intensity=%d \r\n", intensity_enum); Debug(debugChar);
			previous_intensity_enum = intensity_enum;
		}

		if (  ( night_mode_flag == 1)
		    && (   ( TimeSt.Hours >= START_NIGHT_MODE_HOUR  )
			    || ( TimeSt.Hours <  FINISH_NIGHT_MODE_HOUR ))) {
			max7219_init(&h1_max7219, DECODE_MODE, Intensity_1, DISPLAY_DIGIT, WORK_MODE);
		} else {
			max7219_init(&h1_max7219, DECODE_MODE, intensity_enum, DISPLAY_DIGIT, WORK_MODE);
		}
		max7219_show_time( &h1_max7219 , TimeSt.Hours , TimeSt.Minutes );

		ds3231_Alarm1_ClearStatusBit( ADR_I2C_DS3231 );
		Ds3231_hard_alarm_flag_Reset();
		HAL_IWDG_Refresh( &hiwdg );
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
} //***************************************************************************************

void Debug(char* _text) {
	HAL_UART_Transmit(&huart1, (uint8_t *)_text, strlen(_text), 100);
} //***************************************************************************************

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {	//	irqq pin
	if ( GPIO_Pin == BUTTON_1_Pin 	) {
		button_u8 = 1;
	}

	if ( GPIO_Pin == BUTTON_2_Pin 	) {
		button_u8 = 2;
	}

	if ( GPIO_Pin == BUTTON_3_Pin 	) {
		button_u8 = 3;
	}

	if ( GPIO_Pin == BUTTON_4_Pin 	) {
		button_u8 = 4;
	}

	if ( GPIO_Pin == BUTTON_5_Pin 	) {
		button_u8 = 5;
	}

	if ( GPIO_Pin == BUTTON_6_Pin 	) {
		button_u8 = 6;
	}

	if ( GPIO_Pin == DS3231_ALARM_Pin ) {
		Ds3231_hard_alarm_flag_Set();
	}
} //***************************************************************************************

//***************************************************************************************
//***************************************************************************************

/*
**************************************************************************
*                                   END
**************************************************************************
*/
