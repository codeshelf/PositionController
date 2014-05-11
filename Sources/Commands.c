/*
 * Codeshelf
 * � Copyright 2005-2013 Jeffrey B. Williams
 * All rights reserved
 * 
 * $Id$
 * $Name$
 */

#include "Commands.h"
#include <string.h>
#include <Flash.h>

// The bits for the LED segments get loaded into a register via SPI.
// The patter to light the LED element go like this:
// (digit = 1 (right) or 2 (left), elements are A-G and DP)
//     LSB3           LSB2          LSB1            LSB0
// 1F 1A 1B 1DP | 2F 2G 2A 2B | 2DP 2C 2D 2E | 1C 1G 1D 1E

const uint32_t kRightDigitDimBits[] = { 0xfd0000df, 0x5d0000d5, 0x7d00007f, 0x7d0000fd, 0xdd0000f5, 0xf50000fd, 0xf50000ff,
        0x7d0000d5, 0xfd0000ff, 0xfd0000fd };
const uint32_t kLeftDigitDimBits[] = { 0x00df7f00, 0x00577500, 0x007f5f00, 0x007f7d00, 0x00f77500, 0x00fd7d00, 0x00fd7f00,
        0x005f7500, 0x00ff7f00, 0x00ff7d00 };

const uint32_t kRightDigitBlinkBits[] = { 0xa900009a, 0x59000095, 0x6900006a, 0x690000a9, 0x990000a5, 0xa50000a9, 0xa50000aa,
        0x69000095, 0xa90000aa, 0xa90000a9 };
const uint32_t kLeftDigitBlinkBits[] = { 0x009a6a00, 0x00566500, 0x006a5a00, 0x006a6900, 0x00a66500, 0x00a96900, 0x00a96a00,
        0x005a6500, 0x00aa6a00, 0x00aa6900 };

const uint32_t errorDigits = 0xa555556a; // " E"
const uint32_t bayCompDigits = 0x55a56a6a; // "bc"
const uint32_t posAssignDigits = 0x7d5555ff; // " a"

const uint8_t kErrorCode = 255;
const uint8_t kBayCompleteCode = 254;
const uint8_t kPositionAssignCode = 253;

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
void initDisplay() {

	static uint8_t displayInitBytes[] = { 0x12, // Start writing register bytes with register PSC0
	        0x15, // PSC0 - low freq: appears to blink (0.5 sec cycle time)
	        0x40, // PWM0 - 80% duty cycle
	        0x00, // PSC1 - high freq: appears to dim (1/44th sec cycle time)
	        0x40, // PWM1 - 5% duty cycle
	        0x55, // LED 0-3
	        0x55, // LED 4-7
	        0x55, // LED 8-C
	        0x55 // LED C-F
	        };
	static uint16_t bytesSent = 0;

//	error = I2C_SelectSlave(0xC0);
//	error = I2C_SendBlock(displayInitBytes, 10, &bytesSent);
	I2CM_Write_Bytes(0x60, 9, displayInitBytes);
	// Give it some time to write out to the bus.
	Cpu_Delay100US(I2C_DELAY_40MS);

	Flash_GetByteFlash((unsigned int) (&kMyPermanentBusAddr), &gMyBusAddr);

//	for (j = 0; j <= 10; j++) {
//		for (i = 0; i <= 99; i++) {
//			gCurValue = i;
//			displayValue(gCurValue);
//			Cpu_Delay100US(1 * 1000);
//		}
//	}
}

// --------------------------------------------------------------------------
/*
 * When the CHE controller wants to clear clear the display.
 * 
 * Frame format:
 * 1B - command ID
 * 1B - target device bus addr
 */
void clearDisplay() {

	static uint8_t displayBytes[] = { 0x16, // Start writing at register LED 0-3
	        0x55, // LED 0-3
	        0x55, // LED 4-7
	        0x55, // LED 8-B
	        0x55 // LED C-F
	        };

	I2CM_Write_Bytes(0x60, 5, displayBytes);
	// Give it some time to write out to the bus.
	Cpu_Delay100US(I2C_DELAY_40MS);

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

		if (curValue > 99) {
			gCurValue = 0;
			gMinValue = 0;
			gMaxValue = 0;
			displayValueAsCode(curValue);
		} else {
			gCurValue = curValue;
			gMinValue = minValue;
			gMaxValue = gMaxValue;
			displayValue(gCurValue);
		}
	}
}

// --------------------------------------------------------------------------

void displayValue(uint8_t displayValue) {
	static uint8_t displayBytes[] = { 0x14, // Start writing at register LED 0-3
	        0x00, // Frequency
	        0x00, // DUTY Cycle
	        0x00, // LED 0-3
	        0x00, // LED 4-7
	        0x00, // LED 8-B
	        0x00 // LED C-F
	        };
	uint8_t tens;
	uint8_t ones;

	displayBytes[1] = gFreq;
	displayBytes[2] = gDutyCycle;

	tens = displayValue / 10;
	ones = displayValue % 10;

	displayBytes[3] = *(((uint8_t*) &kRightDigitDimBits[ones]) + 0);
	if (tens > 0) {
		displayBytes[4] = *(((uint8_t*) &kLeftDigitDimBits[tens]) + 1);
		displayBytes[5] = *(((uint8_t*) &kLeftDigitDimBits[tens]) + 2);
	} else {
		displayBytes[4] = 0x55;
		displayBytes[5] = 0x55;
	}
	displayBytes[6] = *(((uint8_t*) &kRightDigitDimBits[ones]) + 3);

	I2CM_Write_Bytes(0x60, 7, displayBytes);
	// Give it some time to write out to the bus.
	Cpu_Delay100US(I2C_DELAY_40MS);

}

void displayValueAsCode(uint8_t controlValue) {
	static uint8_t displayBytes[] = { 0x14, // Start writing at register LED 0-3
	        0x00, // Frequency
	        0x00, // DUTY Cycle
	        0x00, // LED 0-3
	        0x00, // LED 4-7
	        0x00, // LED 8-B
	        0x00 // LED C-F
	        };

	displayBytes[1] = gFreq;
	displayBytes[2] = gDutyCycle;

	switch (controlValue) {
		case kErrorCode:
			displayBytes[3] = *(((uint8_t*) &errorDigits) + 0);
			displayBytes[4] = *(((uint8_t*) &errorDigits) + 1);
			displayBytes[5] = *(((uint8_t*) &errorDigits) + 2);
			displayBytes[6] = *(((uint8_t*) &errorDigits) + 3);
			break;
		case kBayCompleteCode:
			displayBytes[3] = *(((uint8_t*) &bayCompDigits) + 0);
			displayBytes[4] = *(((uint8_t*) &bayCompDigits) + 1);
			displayBytes[5] = *(((uint8_t*) &bayCompDigits) + 2);
			displayBytes[6] = *(((uint8_t*) &bayCompDigits) + 3);
			break;
		case kPositionAssignCode:
			displayBytes[3] = *(((uint8_t*) &posAssignDigits) + 0);
			displayBytes[4] = *(((uint8_t*) &posAssignDigits) + 1);
			displayBytes[5] = *(((uint8_t*) &posAssignDigits) + 2);
			displayBytes[6] = *(((uint8_t*) &posAssignDigits) + 3);
			break;
		default:
			break;
	}

	I2CM_Write_Bytes(0x60, 7, displayBytes);
	// Give it some time to write out to the bus.
	Cpu_Delay100US(I2C_DELAY_40MS);
}

// --------------------------------------------------------------------------

void displayValueBlink(uint8_t displayValue) {

	static uint8_t displayBytes[] = { 0x14, // Start writing at register LED 0-3
	        0x00, // Frequency
	        0x00, // DUTY Cycle
	        0x00, // LED 0-3
	        0x00, // LED 4-7
	        0x00, // LED 8-B
	        0x00 // LED C-F
	        };

	uint8_t tens;
	uint8_t ones;

	displayBytes[1] = gFreq;
	displayBytes[2] = gDutyCycle;

	tens = displayValue / 10;
	ones = displayValue % 10;

	displayBytes[3] = *(((uint8_t*) &kRightDigitBlinkBits[ones]) + 0);
	if (tens > 0) {
		displayBytes[4] = *(((uint8_t*) &kLeftDigitBlinkBits[tens]) + 1);
		displayBytes[5] = *(((uint8_t*) &kLeftDigitBlinkBits[tens]) + 2);
	} else {
		displayBytes[4] = 0x55;
		displayBytes[5] = 0x55;
	}
	displayBytes[6] = *(((uint8_t*) &kRightDigitBlinkBits[ones]) + 3);

	I2CM_Write_Bytes(0x60, 7, displayBytes);
	// Give it some time to write out to the bus.
	Cpu_Delay100US(I2C_DELAY_40MS);

}
