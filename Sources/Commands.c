/*
 * Codeshelf
 * © Copyright 2005-2013 Jeffrey B. Williams
 * All rights reserved
 * 
 * $Id$
 * $Name$
 */

#include "Commands.h"
#include <string.h>
#include <Flash.h>

uint8_t gMessageBuffer[MAX_FRAME_BYTES];
uint32_t gRightDigitBits[] = { 0x01000010, 0x51000015, 0x41000040, 0x41000001, 0x11000005, 0x05000001, 0x05000000, 0x41000015, 0x01000000, 0x01000001 };
uint32_t gLeftDigitBits[] =  { 0x00104000, 0x00544500, 0x00405000, 0x00404100, 0x00044500, 0x00014100, 0x00014000, 0x00504500, 0x00004000, 0x00004500 };
uint8_t gCurValue = 0;
uint8_t gMinValue = 0;
uint8_t gMaxValue = 0;

// --------------------------------------------------------------------------

void processFrame(FramePtrType framePtr, FrameCntType frameByteCount) {
	
	uint8_t myBusId = 0x0;
	if (Flash_GetByteFlash(0x0, &myBusId) == ERR_OK) {

		// Dispatch the command if the bus ID is zero or matches our bus ID.
		if ((myBusId == framePtr[COMMAND_BUSID_POS]) || (0 == framePtr[COMMAND_BUSID_POS])) {
			switch (framePtr[COMMANDID_POS]) {
				case INIT_COMMAND:
					initDisplay();
					break;
				case CLEAR_COMMAND:
					clearDisplay();
					break;
				case DISPLAY_COMMAND:
					setValues(framePtr, frameByteCount);
					break;
				default:
					break;
			}
		}
	}
}

// --------------------------------------------------------------------------
/*
 * When the CHE controller initializes it sends the init command.
 * 
 * Frame format:
 * 1B - command ID
 * 8B - CHE ID
 */
void initDisplay() {
	static uint8_t displayInitBytes[] = { 0x12, 0x00, 0xb8, 0x00, 0xb8, 0x55, 0x55, 0x55, 0x55 };
	static uint16_t bytesSent = 0;
	uint8_t error;
	uint8_t i;
	uint8_t j;

//	error = I2C_SelectSlave(0xC0);
//	error = I2C_SendBlock(displayInitBytes, 10, &bytesSent);
	I2CM_Write_Bytes(0x60, 9, displayInitBytes);
	
	for (j = 0; j <= 10; j++) {
		for (i = 0; i <= 99; i++) {
			gCurValue = i;
			displayCurrentValue();
			Cpu_Delay100US(1 * 1000);
		}
	}
}

// --------------------------------------------------------------------------
/*
 * When the CHE controller wants to clear clear the display.
 * 
 * Frame format:
 * 1B - command ID
 * 1B - position number
 */
void clearDisplay() {

	// Clear the display .
	initDisplay();

}

// --------------------------------------------------------------------------
/*
 * When the CHE controller intialize the display value and the show it.
 * 
 * Frame format:
 * 1B - command ID
 * 2B - quantity to display
 * 2B - max quantity (usually same as quantity to display)
 * 2B - min quantity (usually zero)
 */
void setValues(FramePtrType framePtr, FrameCntType frameByteCount) {

	uint8_t myBusId = 0x0;
	if (Flash_GetByteFlash(0x0, &myBusId) == ERR_OK) {

		if (myBusId == framePtr[COMMAND_BUSID_POS]) {
			gCurValue = framePtr[DISPLAY_CMD_VAL_POS];
			gMinValue = framePtr[DISPLAY_CMD_MIN_POS];
			gMaxValue = framePtr[DISPLAY_CMD_MAX_POS];

			displayCurrentValue();
		}
	}}

// --------------------------------------------------------------------------

void displayCurrentValue() {

	static uint8_t displayBytes[] = { 0x16, 0x00, 0x00, 0x00, 0x00 };
	static uint16_t bytesSent;
	uint8_t tens;
	uint8_t ones;
	uint8_t error;
	uint8_t i;
	uint8_t value;
	uint8_t *ptr;
	
	tens = gCurValue / 10;
	ones = gCurValue % 10;

	displayBytes[1] = *(((uint8_t*) &gRightDigitBits[ones]) + 0);
	if (tens > 0) {
		displayBytes[2] = *(((uint8_t*) &gLeftDigitBits[tens]) + 1);
		displayBytes[3] = *(((uint8_t*) &gLeftDigitBits[tens]) + 2);
	} else {
		displayBytes[2] = 0x55;
		displayBytes[3] = 0x55;
	}
	displayBytes[4] = *(((uint8_t*) &gRightDigitBits[ones]) + 3);
	             
//		error = I2C_SelectSlave(0xC0);
//		error = I2C_SendBlock(displayBytes, 5, &bytesSent);
	I2CM_Write_Bytes(0x60, 5, displayBytes);

}
