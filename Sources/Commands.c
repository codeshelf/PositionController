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

uint8_t gMessageBuffer[MAX_FRAME_BYTES];
uint16_t gRightDigitBits[] = { 0x6aa6, 0x6556, 0x69a9, 0x696a, 0x665a, 0x5a6a, 0x5aaa, 0x6956, 0x6aaa, 0x6a5a };
uint16_t gLeftDigitBits[]  = { 0x5555, 0x5995, 0xa5a9, 0x69a9, 0x599a, 0x696a, 0xa96a, 0x59a5, 0xa9aa, 0x59aa };
uint8_t gCurValue = 0;
uint8_t gMinValue = 0;
uint8_t gMaxValue = 0;

// --------------------------------------------------------------------------

void processFrame(FramePtrType framePtr, FrameCntType frameByteCount) {

	// Dispatch the command if the bus ID is zero or matches our bus ID.
	if ((MYBUSID == framePtr[COMMAND_BUSID_POS]) || (0 == framePtr[COMMAND_BUSID_POS])) {
		switch (framePtr[COMMANDID_POS]) {
			case INIT_COMMAND:
				init(framePtr, frameByteCount);
				break;
			case CLEAR_COMMAND:
				clear(framePtr, frameByteCount);
				break;
			case DISPLAY_COMMAND:
				display(framePtr, frameByteCount);
				break;
			default:
				break;
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
void init(FramePtrType framePtr, FrameCntType frameByteCount) {
	static uint8_t displayInitBytes[] = { 0x12, 0x00, 0xb8, 0x00, 0xb8, 0x55, 0x55, 0x55, 0x55 };
	static uint16_t bytesSent = 0;
	uint8_t error;

//	error = I2C_SelectSlave(0xC0);
//	error = I2C_SendBlock(displayInitBytes, 10, &bytesSent);
	I2CM_Write_Bytes(0x60, 9, displayInitBytes);
}

/*
 * When the CHE controller wants to clear the display(s) command.
 * 
 * Frame format:
 * 1B - command ID
 * 1B - position number
 */
// --------------------------------------------------------------------------
void clear(FramePtrType framePtr, FrameCntType frameByteCount) {

	// Clear the display .
	init(framePtr, frameByteCount);

}

/*
 * When the CHE controller initializes it sends the init command.
 * 
 * Frame format:
 * 1B - command ID
 * 2B - quantity to display
 * 2B - max quantity (usually same as quantity to display)
 * 2B - min quantity (usually zero)
 */
// --------------------------------------------------------------------------
void display(FramePtrType framePtr, FrameCntType frameByteCount) {

	static uint8_t displayBytes[] = { 0x16, 0x00, 0x00, 0x00, 0x00 };
	static uint16_t bytesSent;
	uint8_t tens;
	uint8_t ones;
	uint8_t error;

	if (MYBUSID == framePtr[COMMAND_BUSID_POS]) {
		gCurValue = framePtr[DISPLAY_CMD_VAL_POS];
		gMinValue = framePtr[DISPLAY_CMD_MIN_POS];
		gMaxValue = framePtr[DISPLAY_CMD_MAX_POS];

		tens = gCurValue / 10;
		ones = gCurValue % 10;

		memcpy2(&displayBytes[1], &gLeftDigitBits[tens], 2);
		memcpy2(&displayBytes[3], &gRightDigitBits[ones], 2);

//		error = I2C_SelectSlave(0xC0);
//		error = I2C_SendBlock(displayBytes, 5, &bytesSent);
		I2CM_Write_Bytes(0x60, 5, displayBytes);
	}
}
