/**
* \file
* \version 1.0
* \author bazhen.levkovets
* \date 2021
* \mail bazhen.info(at)gmail.com
*************************************************************************************
* \copyright	Bazhen Levkovets
* \copyright	Brovary, Kyiv region
* \copyright	Ukraine
*
*
*************************************************************************************
*
* \brief
*
*/

#ifndef DIGITAL_CLOCK_FOR_BEDROOM_INCLUDED_H_
#define DIGITAL_CLOCK_FOR_BEDROOM_INCLUDED_H_

	
	/*
	**************************************************************************
	*								INCLUDE FILES
	**************************************************************************
	*/
	
	#include "stdio.h"
	#include <string.h>

	#include "main.h"
	#include "i2c.h"
	#include "rtc.h"
	#include "spi.h"
	#include "usart.h"
	#include "iwdg.h"

	#include "ds3231_local_config.h"
	#include "max7219_local_config.h"
	#include "digital_clock_for_bedroom_local_config.h"
	#include "i2c_techmaker_sm.h"
	#include "ds3231_sm.h"
	#include "max7219_4x_dot_sm.h"

	/*
	**************************************************************************
	*								    DEFINES                     
	**************************************************************************
	*/
	
	/*
	**************************************************************************
	*								   DATA TYPES
	**************************************************************************
	*/
	
	/*
	**************************************************************************
	*								GLOBAL VARIABLES
	**************************************************************************
	*/

	/*
	**************************************************************************
	*									 MACRO'S                     
	**************************************************************************
	*/

	/*
	**************************************************************************
	*                              FUNCTION PROTOTYPES
	**************************************************************************
	*/
	
	void Digit_clock_Init (void)			;
	void Digit_clock_Main (void)			;
	void Set_ds3231_alarm (void) 			;

#endif /* DIGITAL_CLOCK_FOR_BEDROOM_INCLUDED_H_ */
