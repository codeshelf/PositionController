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

// LED status values
uint8_t gLedRedValue = 0;
uint8_t gLedGreenValue = 0;
uint8_t gLedBlueValue = 0;
uint8_t gLedLightStyle = 0;

extern bool gAckButtonLockout;

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
				startMassConfigMode();
				break;
			case IDSETUPINC_COMMAND:
				incrementMassConfigMode(framePtr, frameByteCount);
				break;
			case BUTTONCREATE_COMMAND:
				createButtonPress(framePtr, frameByteCount);
				break;
			case DISPLAY_ADDR_COMMAND:
				gDeviceState = eAddressDisplayMode;
				displayBusAddress();
				break;
			case SET_LED:
				setLedValues(framePtr, frameByteCount);
				break;
			case DISPLAY_FWVER_COMMAND:
				gDeviceState = eFirmwareDisplayMode;
				displayFirmwareVersion();
				break;
			default:
				break;
		}
	}
}

// --------------------------------------------------------------------------
void displayBusAddress() {
	initDisplay();

	if (gMyBusAddr == UNSET_BUSADDR) {
		// Flash 0x00 if we have no bus address
		displayValueBlink(0x00);
	} else {
		// Display current bus address
		displayValue(gMyBusAddr);
	}
}

// --------------------------------------------------------------------------
void createButtonPress(FramePtrType framePtr, FrameCntType frameByteCount) {
	if ((gMyBusAddr == framePtr[COMMAND_BUSADDR_POS]) || (BROADCAST_BUSADDR == framePtr[COMMAND_BUSADDR_POS])) {

		gCurValue = framePtr[COMMAND_CREATE_NUM];

		Cpu_Delay100US(1 * 1000);
		sendAckCommand();
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

		clearDisplay();
		if (curValue <= 99) {
			gCurValue = curValue;
			gMinValue = minValue;
			gMaxValue = maxValue;
			displayValue(gCurValue);
		} else if (curValue == kLedSegmentsCode) {
			gMinValue = 255;
			gMaxValue = 255;
			gCurValue = 255;
			setLedSegments(minValue, maxValue);
		}
	}
}

// --------------------------------------------------------------------------

void startMassConfigMode() {
	gDeviceState = eMassConfigMode;
	gCurValue = 0x01;
	gMinValue = 1;
	gMaxValue = 99;
	displayValueBlink(gCurValue);
}

// --------------------------------------------------------------------------

void incrementMassConfigMode(FramePtrType framePtr, FrameCntType frameByteCount) {
	if (gDeviceState == eMassConfigMode) {
		gCurValue = framePtr[IDSETUPINC_COMMAND_NUM_POS];
		displayValueBlink(gCurValue);
	}
}

// --------------------------------------------------------------------------

void sendIdSetupIncCommand(uint8_t n) {
	uint8_t commandBytes[] = { IDSETUPINC_COMMAND, 0x00, 0};
	commandBytes[2] = n+1;

	serialTransmitFrame((FramePtrType) &commandBytes, 3);
}

// --------------------------------------------------------------------------

void sendAckCommand() {
	uint8_t commandBytes[] = { BUTTON_COMMAND, 0x00, 0x00 };

	commandBytes[COMMAND_BUSADDR_POS] = gMyBusAddr; //kMyPermanentBusAddr;
	commandBytes[BUTTON_CMD_DATA_POS] = gCurValue;

	serialTransmitFrame((FramePtrType) &commandBytes, 3);
}

// --------------------------------------------------------------------------

void setLedValues(FramePtrType framePtr, FrameCntType frameByteCount) {
	
	gLedRedValue = framePtr[DISPLAY_CMD_RED_POS];
	gLedGreenValue = framePtr[DISPLAY_CMD_GREEN_POS];
	gLedBlueValue = framePtr[DISPLAY_CMD_BLUE_POS];
	gLedLightStyle = framePtr[DISPLAY_CMD_SYTLE_POS];
}
