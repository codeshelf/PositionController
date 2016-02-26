/*
 * StatusLedWs2811.c
 *
 *  Created on: Apr 4, 2015
 *      Author: jeffw
 */

#include "StatusLed.h"
#include "IO_Map.h"
#include "StatusLedClkDo.h"
#include "Wait.h"

extern uint8_t gLedRedValue;
extern uint8_t gLedGreenValue;
extern uint8_t gLedBlueValue;
extern uint8_t gLedLightStyle;

void setStatusLedValues(uint8_t red, uint8_t green, uint8_t blue, uint8_t style) {
	gLedRedValue = red;
	gLedGreenValue = green;
	gLedBlueValue = blue;
	gLedLightStyle = style;
}
