/*
 * Codeshelf
 * © Copyright 2005-2013 Jeffrey B. Williams
 * All rights reserved
 * 
 * $Id$
 * $Name$
 */

#include "Commands.h"
#include "Display.h"
#include "Uart.h"

// This value is set in flash, but it gets overwritten by the user config mode.
#pragma CONST_SEG FLASH_STORAGE  
const uint8_t kMyPermanentBusAddr = UNSET_BUSADDR;
#pragma CONST_SEG DEFAULT  

// Send the bit-encoded LED segments in the min and max bytes.
const uint8_t kLedSegmentsCode = 240;

EDeviceState gDeviceState = eInactive;
uint8_t gMyBusAddr;
uint8_t gMessageBuffer[MAX_FRAME_BYTES];
uint8_t gCurValue = 0;
uint8_t gMinValue = 0;
uint8_t gMaxValue = 0;

uint8_t gFreq = 0;
uint8_t gDutyCycle;

// --------------------------------------------------------------------------

void processFrame(FramePtrType framePtr, FrameCntType frameByteCount) {

	// Dispatch the command if the bus ID is zero or matches our bus ID.
	if ((gMyBusAddr == framePtr[COMMAND_BUSADDR_POS]) || (BROADCAST_BUSADDR == framePtr[COMMAND_BUSADDR_POS])) {
		switch (framePtr[COMMANDID_POS]) {
			case INIT_COMMAND:
				gDeviceState = eInactive;
				initDisplay();
				break;
			case CLEAR_COMMAND:
				gDeviceState = eInactive;
				clearDisplay();
				break;
			case DISPLAY_COMMAND:
				gDeviceState = eActive;
				setValues(framePtr, frameByteCount);
				break;
			case QUERY_COMMAND:
				break;
			case IDSETUPSTART_COMMAND:
				startConfigMode();
				break;
			case IDSETUPINC_COMMAND:
				incrementConfigMode();
				break;
			default:
				break;
		}
	}
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

	if ((gMyBusAddr == framePtr[COMMAND_BUSADDR_POS]) || (BROADCAST_BUSADDR == framePtr[COMMAND_BUSADDR_POS])) {
		uint8_t curValue = framePtr[DISPLAY_CMD_VAL_POS];
		uint8_t minValue = framePtr[DISPLAY_CMD_MIN_POS];
		uint8_t maxValue = framePtr[DISPLAY_CMD_MAX_POS];
		uint8_t freq = framePtr[DISPLAY_CMD_FREQ_POS];
		uint8_t dutyCycle = framePtr[DISPLAY_CMD_DUTY_POS];

		gFreq = freq;
		gDutyCycle = dutyCycle;

		if (curValue <= 99) {
			gCurValue = curValue;
			gMinValue = minValue;
			gMaxValue = maxValue;
			displayValue(gCurValue);
		} else if (curValue == kLedSegmentsCode) {
			gMinValue = 0;
			gMaxValue = 0;
			gCurValue = 0;
			setLedSegments(minValue, maxValue);
		}
	}
}

// --------------------------------------------------------------------------

void startConfigMode() {
	gDeviceState = eConfigMode;
	gCurValue = 0x01;
	gMinValue = 1;
	gMaxValue = 99;
	displayValueBlink(gCurValue);
}

// --------------------------------------------------------------------------

void incrementConfigMode() {
	if (gDeviceState == eConfigMode) {
		gCurValue++;
		displayValueBlink(gCurValue);
	}
}

// --------------------------------------------------------------------------

void sendIdSetupIncCommand() {
	uint8_t commandBytes[] = { IDSETUPINC_COMMAND, 0x00 };

	serialTransmitFrame((FramePtrType) &commandBytes, 2);
}

// --------------------------------------------------------------------------

void sendAckCommand() {
	uint8_t commandBytes[] = { BUTTON_COMMAND, 0x00, 0x00 };

	commandBytes[1] = kMyPermanentBusAddr;
	commandBytes[BUTTON_CMD_DATA_POS] = gCurValue;

	serialTransmitFrame((FramePtrType) &commandBytes, 3);
}


