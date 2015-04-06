/*
 * StatusLed.c
 *
 *  Created on: Apr 4, 2015
 *      Author: jeffw
 */

#include "StatusLed.h"

void setStatusLed(uint8_t red, uint8_t green, uint8_t blue) {
	uint8_t bitpos;
	uint8_t ccrHolder;
	
	// Red
	for (bitpos = 128; bitpos > 0; bitpos>>=1) {
		if (red & bitpos) {
			StatusLedSdi_SetVal();
		} else {
			StatusLedSdi_ClrVal();
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
			StatusLedSdi_SetVal();
		} else {
			StatusLedSdi_ClrVal();
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
			StatusLedSdi_SetVal();
		} else {
			StatusLedSdi_ClrVal();
		}
		Wait_Waitus(1);
		StatusLedClk_SetVal();
		Wait_Waitus(1);
		StatusLedClk_ClrVal();
		Wait_Waitus(1);
	}
}
