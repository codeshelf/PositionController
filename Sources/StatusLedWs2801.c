/*
 * StatusLed.c
 *
 *  Created on: Apr 4, 2015
 *      Author: jeffw
 */

#include "StatusLed.h"
#include "StatusLedData.h"
#include "StatusLedClk.h"
#include "Wait.h"

void setStatusLed(uint8_t red, uint8_t green, uint8_t blue) {
	uint8_t bitpos;
	
	// Red
	for (bitpos = 128; bitpos > 0; bitpos>>=1) {
		if (red & bitpos) {
			StatusLedData_SetVal();
		} else {
			StatusLedData_ClrVal();
		}
		Wait_Waitus(1);
		StatusLedClk_SetVal();
		Wait_Waitus(1);
		StatusLedClk_ClrVal();
		Wait_Waitus(1);
	}

	// Green
	for (bitpos = 128; bitpos > 0; bitpos>>=1) {
		if (green & bitpos) {
			StatusLedData_SetVal();
		} else {
			StatusLedData_ClrVal();
		}
		Wait_Waitus(1);
		StatusLedClk_SetVal();
		Wait_Waitus(1);
		StatusLedClk_ClrVal();
		Wait_Waitus(1);
	}

	// Blue
	for (bitpos = 128; bitpos > 0; bitpos>>=1) {
		if (blue & bitpos) {
			StatusLedData_SetVal();
		} else {
			StatusLedData_ClrVal();
		}
		Wait_Waitus(1);
		StatusLedClk_SetVal();
		Wait_Waitus(1);
		StatusLedClk_ClrVal();
		Wait_Waitus(1);
	}
}
