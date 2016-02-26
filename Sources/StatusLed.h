/*
 * StatusLed.h
 *
 *  Created on: Apr 4, 2015
 *      Author: jeffw
 */

#ifndef STATUSLED_H_
#define STATUSLED_H_

#include "PE_Types.h"
#include "StatusLed.h"
#include "IO_Map.h"
#include "StatusLedClkDo.h"
#include "Wait.h"

void setStatusLedValues(uint8_t red, uint8_t green, uint8_t blue, uint8_t style);

extern uint8_t gLedRedValue;
extern uint8_t gLedGreenValue;
extern uint8_t gLedBlueValue;

#endif /* STATUSLED_H_ */
