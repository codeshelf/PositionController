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
#include "DisplayLed.h"
#include "FirmwareVersion.h"

extern uint8_t gCurValue;
extern uint8_t gMinValue;
extern uint8_t gMaxValue;
extern uint8_t kMyPermanentBusAddr;
extern uint8_t gMyBusAddr;
extern EDeviceState gDeviceState;

extern uint8_t gFreq;
extern uint8_t gDutyCycle;

// The bits for the LED segments get loaded into a register via SPI.
// The pattern to light the LED element go like this:
// In the code below the number is the digit = 1 (right) or 2 (left), and the letter is the standard LED segment.
// https://en.wikipedia.org/wiki/File:7_segment_display_labeled.svg
//
//     LSB3           LSB2          LSB1          LSB0
// 1DP 1C 1D 1E | 2DP 2C 2D 2E | 2G 2F 2A 2B | 1G 1F 1A 1B 
//
// LSBx refers to the 9552's LED register number.  
// We send them in LSB0, LSB1, LSB2, LSB3 order on the IIC bus.
// 
// In the segment position, e.g. 1F, use one of the follow 2-bit codes:
//
// 00 = LED Output set LOW (LED On)
// 01 = LED Output set Hi-Z (LED Off – Default) 
// 10 = LED Output blinks at BLINK0 Rate
// 11 = LED Output blinks at BLINK1 Rate

const uint16_t kRightDigitBlinkBits[] = { 0x6a6a, 0x5665, 0x9a5a, 0x9a69, 0xa665,
		0xa969, 0xa96a, 0x5a65, 0xaa6a, 0xaa65 };
const uint16_t kLeftDigitBlinkBits[] =  { 0x6a6a, 0x5665, 0x9a5a, 0x9a69, 0xa665,
		0xa969, 0xa96a, 0x5a65, 0xaa6a, 0xaa65 };

const uint16_t kRightDigitDimBits[] = { 0x7f7f, 0x5775, 0xdf5f, 0xdf7d, 0xf775,
		0xfd7d, 0xfd7f, 0x5f75, 0xff7f, 0xff75 };
const uint16_t kLeftDigitDimBits[] =  { 0x7f7f, 0x5775, 0xdf5f, 0xdf7d, 0xf775,
		0xfd7d, 0xfd7f, 0x5f75, 0xff7f, 0xff75 };

const uint8_t kSegmentMap[] = {
		// 1A (0) to 1DP (7)
		5, 4, 10, 9, 8, 6, 7, 11,
        // 2A (8) to 2DP (15)
		1, 0, 14, 13, 12, 2, 3, 15 };

const uint8_t kSegementOffBits = 0b00000001;
const uint8_t kSegementDimBits = 0b00000010;

uint8_t gDisplayBytes[12];

// --------------------------------------------------------------------------
/*
 * When the CHE controller initializes it sends the init command.
 * 
 * Frame format:
 * 1B - command ID
 * 8B - CHE ID
 */
void initDisplay() {

	static uint16_t bytesSent = 0;
	
	gDisplayBytes[0] = 0x12; // Start writing register bytes with register PSC0
	gDisplayBytes[1] = 0x15; // PSC0 - low freq: appears to blink (0.5 sec cycle time)
	gDisplayBytes[2] = 0x40; // PWM0 - 80% duty cycle
	gDisplayBytes[3] = 0x00; // PSC1 - high freq: appears to dim (1/44th sec cycle time)
	gDisplayBytes[4] = 0x40; // PWM1 - 5% duty cycle
	gDisplayBytes[5] = 0x55; // LED 0-3
	gDisplayBytes[6] = 0x55; // LED 4-7
	gDisplayBytes[7] = 0x55; // LED 8-C
	gDisplayBytes[8] = 0x55; // LED C-F

//	error = I2C_SelectSlave(0xC0);
//	error = I2C_SendBlock(displayInitBytes, 10, &bytesSent);
	I2CM_Write_Bytes(0x60, 9, gDisplayBytes);
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

	gDisplayBytes[0] = 0x16; // Start writing register bytes with register PSC0
	gDisplayBytes[1] = 0x55; // LED 0-3
	gDisplayBytes[2] = 0x55; // LED 4-7
	gDisplayBytes[3] = 0x55; // LED 8-C
	gDisplayBytes[4] = 0x55; // LED C-F

	I2CM_Write_Bytes(0x60, 5, gDisplayBytes);
	// Give it some time to write out to the bus.
	Cpu_Delay100US(I2C_DELAY_40MS);

}

// --------------------------------------------------------------------------

void displayValue(uint8_t displayValue) {
	uint8_t tens;
	uint8_t ones;

	gDisplayBytes[0] = 0x14; // Start writing register bytes with register PSC0
	gDisplayBytes[1] = gFreq; // PSC1 - high freq: appears to dim (1/44th sec cycle time)
	gDisplayBytes[2] = gDutyCycle; // PWM1 - 5% duty cycle
	gDisplayBytes[3] = 0x55; // LED 0-3
	gDisplayBytes[4] = 0x55; // LED 4-7
	gDisplayBytes[5] = 0x55; // LED 8-C
	gDisplayBytes[6] = 0x55; // LED C-F

	tens = displayValue / 10;
	ones = displayValue % 10;

	gDisplayBytes[4] = *(((uint8_t*) &kRightDigitDimBits[ones]) + 0);
	gDisplayBytes[5] = *(((uint8_t*) &kRightDigitDimBits[ones]) + 1);
	if (tens > 0) {
		gDisplayBytes[3] = *(((uint8_t*) &kLeftDigitDimBits[tens]) + 0);
		gDisplayBytes[6] = *(((uint8_t*) &kLeftDigitDimBits[tens]) + 1);
	} else {
		gDisplayBytes[3] = 0x55;
		gDisplayBytes[6] = 0x55;
	}

	I2CM_Write_Bytes(0x60, 7, gDisplayBytes);
	// Give it some time to write out to the bus.
	Cpu_Delay100US(I2C_DELAY_40MS);
}

// --------------------------------------------------------------------------

void setLedSegments(uint8_t firstDigitVal, uint8_t secondDigitVal) {

	uint8_t bitPos;
	uint8_t mappedBitPos;
	bool segmentIsOn;
	uint8_t bytePos;
	uint8_t shiftBits;
	uint8_t bitVal;

	gDisplayBytes[0] = 0x12; // Start writing register bytes with register PSC0
	gDisplayBytes[1] = gFreq; // PSC0 - low freq: appears to blink (0.5 sec cycle time)
	gDisplayBytes[2] = gDutyCycle; // PWM0 - 80% duty cycle
	gDisplayBytes[3] = gFreq; // PSC1 - high freq: appears to dim (1/44th sec cycle time)
	gDisplayBytes[4] = gDutyCycle; // PWM1 - 5% duty cycle
	gDisplayBytes[5] = 0x55; // LED 0-3
	gDisplayBytes[6] = 0x55; // LED 4-7
	gDisplayBytes[7] = 0x55; // LED 8-C
	gDisplayBytes[8] = 0x55; // LED C-F

	// Loop through the bits and set the segments.
	for (bitPos = 0; bitPos < 16; ++bitPos) {
		segmentIsOn = FALSE;
		if (bitPos < 8) {
			if (firstDigitVal & (0x01 << bitPos)) {
				segmentIsOn = TRUE;
			}
		} else {
			if (secondDigitVal & (0x01 << (bitPos - 8))) {
				segmentIsOn = TRUE;
			}
		}

		mappedBitPos = kSegmentMap[bitPos];
		bytePos = (uint8_t) (mappedBitPos / 4);
		shiftBits = (uint8_t)((mappedBitPos % 4) * 2);
		if (segmentIsOn) {
			bitVal = (uint8_t) (kSegementDimBits << shiftBits);
		} else {
			bitVal = (uint8_t) (kSegementOffBits << shiftBits);
		}
		gDisplayBytes[5 + bytePos] |= bitVal;
	}
	
	I2CM_Write_Bytes(0x60, 9, gDisplayBytes);
	// Give it some time to write out to the bus.
	Cpu_Delay100US(I2C_DELAY_40MS);
}

// --------------------------------------------------------------------------

void displayValueBlink(uint8_t displayValue) {

	uint8_t tens;
	uint8_t ones;

	gDisplayBytes[0] = 0x14; // Start writing register bytes with register PSC0
	gDisplayBytes[1] = gFreq; // PSC1 - high freq: appears to dim (1/44th sec cycle time)
	gDisplayBytes[2] = gDutyCycle; // PWM1 - 5% duty cycle
	gDisplayBytes[3] = 0x55; // LED 0-3
	gDisplayBytes[4] = 0x55; // LED 4-7
	gDisplayBytes[5] = 0x55; // LED 8-C
	gDisplayBytes[6] = 0x55; // LED C-F

	tens = displayValue / 10;
	ones = displayValue % 10;
	
	gDisplayBytes[4] = *(((uint8_t*) &kRightDigitBlinkBits[ones]) + 0);
	gDisplayBytes[5] = *(((uint8_t*) &kRightDigitBlinkBits[ones]) + 1);
	if (tens > 0) {
		gDisplayBytes[3] = *(((uint8_t*) &kLeftDigitBlinkBits[tens]) + 0);
		gDisplayBytes[6] = *(((uint8_t*) &kLeftDigitBlinkBits[tens]) + 1);
	} else {
		gDisplayBytes[3] = 0x55;
		gDisplayBytes[6] = 0x55;
	}

	I2CM_Write_Bytes(0x60, 7, gDisplayBytes);
	// Give it some time to write out to the bus.
	Cpu_Delay100US(I2C_DELAY_40MS);

}

void displayFirmwareVersion() {
	uint8_t tens;
	uint8_t ones;

	gDisplayBytes[0] = 0x14; // Start writing register bytes with register PSC0
	gDisplayBytes[1] = gFreq; // PSC1 - high freq: appears to dim (1/44th sec cycle time)
	gDisplayBytes[2] = gDutyCycle; // PWM1 - 5% duty cycle
	gDisplayBytes[3] = 0x55; // LED 0-3
	gDisplayBytes[4] = 0x55; // LED 4-7
	gDisplayBytes[5] = 0x55; // LED 8-C
	gDisplayBytes[6] = 0x55; // LED C-F

	tens = FWVERSION_MAJOR;
	ones = FWVERSION_MINOR;
	
	gDisplayBytes[4] = *(((uint8_t*) &kRightDigitBlinkBits[ones]) + 0);
	gDisplayBytes[5] = *(((uint8_t*) &kRightDigitBlinkBits[ones]) + 1);
	if (tens > 0) {
		gDisplayBytes[3] = *(((uint8_t*) &kLeftDigitBlinkBits[tens]) + 0);
		gDisplayBytes[6] = *(((uint8_t*) &kLeftDigitBlinkBits[tens]) + 1);
	} else {
		gDisplayBytes[3] = 0x55;
		gDisplayBytes[6] = 0x55;
	}
	
	// Set the decimal point LED element to 10;
	gDisplayBytes[6] |= 0x80;
	gDisplayBytes[6] &= 0xbf;

	I2CM_Write_Bytes(0x60, 7, gDisplayBytes);
	// Give it some time to write out to the bus.
	Cpu_Delay100US(I2C_DELAY_40MS);
}

