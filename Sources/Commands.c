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

EDeviceState gDeviceState = eInactive;
uint8_t gMyBusAddr;
uint8_t gMessageBuffer[MAX_FRAME_BYTES];
uint8_t gCurValue = 0;
uint8_t gMinValue = 0;
uint8_t gMaxValue = 0;

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


