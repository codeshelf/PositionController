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
#include "Led.h"

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

const uint32_t errorDigits = 0xa955555a; // " E"
const uint32_t bayCompDigits = 0xf5d55f7f; // "bc"
const uint32_t posAssignDigits = 0xdf55557f; // " a"
const uint32_t posRepeatDigits = 0xd5555557; // " r"

const uint8_t kErrorCode = 255;
const uint8_t kBayCompleteCode = 254;
const uint8_t kPositionAssignCode = 253;
const uint8_t kPositionRepeatCode = 252;

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
	if ((gMyBusAddr == framePtr[COMMAND_BUSADDR_POS])
			|| (BROADCAST_BUSADDR == framePtr[COMMAND_BUSADDR_POS])) {
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

	if ((gMyBusAddr == framePtr[COMMAND_BUSADDR_POS])
			|| (BROADCAST_BUSADDR == framePtr[COMMAND_BUSADDR_POS])) {
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

	displayBytes[4] = *(((uint8_t*) &kRightDigitDimBits[ones]) + 0);
	displayBytes[5] = *(((uint8_t*) &kRightDigitDimBits[ones]) + 1);
	if (tens > 0) {
		displayBytes[3] = *(((uint8_t*) &kLeftDigitDimBits[tens]) + 0);
		displayBytes[6] = *(((uint8_t*) &kLeftDigitDimBits[tens]) + 1);
	} else {
		displayBytes[3] = 0x55;
		displayBytes[6] = 0x55;
	}

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
	case kPositionRepeatCode:
		displayBytes[3] = *(((uint8_t*) &posRepeatDigits) + 0);
		displayBytes[4] = *(((uint8_t*) &posRepeatDigits) + 1);
		displayBytes[5] = *(((uint8_t*) &posRepeatDigits) + 2);
		displayBytes[6] = *(((uint8_t*) &posRepeatDigits) + 3);
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
	
	displayBytes[4] = *(((uint8_t*) &kRightDigitBlinkBits[ones]) + 0);
	displayBytes[5] = *(((uint8_t*) &kRightDigitBlinkBits[ones]) + 1);
	if (tens > 0) {
		displayBytes[3] = *(((uint8_t*) &kLeftDigitBlinkBits[tens]) + 0);
		displayBytes[6] = *(((uint8_t*) &kLeftDigitBlinkBits[tens]) + 1);
	} else {
		displayBytes[3] = 0x55;
		displayBytes[6] = 0x55;
	}

	I2CM_Write_Bytes(0x60, 7, displayBytes);
	// Give it some time to write out to the bus.
	Cpu_Delay100US(I2C_DELAY_40MS);

}

#define		ON		\
		asm			MOV #0b00001000, 2;	\
		asm			NOP;				\
		asm			NOP;				\
		asm			MOV #0b00000000, 2;

#define		OFF		\
		asm			MOV #0b00001000, 2;	\
		asm			MOV #0b00000000, 2;	

void setLedIndicator(uint8_t blueValue, uint8_t greeValue, uint8_t redValue) {

asm {
	// Param 1 (blue) on stack
	// Param 2 (green) in X
	// Param 3 (red) in A
	
	// Push 2rd param
		PSHA
		
	// Green
	// Pop A back off the stack
		TXA
		LDX    #8
	Loop2:
		ASRA
		BCS    On2
	Off2:
		PSHA
		LDA    #0
		MOV    #PTBD_PTBD4_MASK,0x02
		//MOV    #0x00,0x02
		STA    0x02
		PULA
		BRA    End2
	On2:
		MOV    #PTBD_PTBD4_MASK,0x02
		NOP    
		NOP    
		MOV    #0x00,0x02
	End2:
		DECX
		BNE    Loop2
	
	// Red
		PULA
		LDX    #8
	Loop1:
		ASRA
		BCS    On1
	Off1:
		PSHA
		LDA    #0
		MOV    #PTBD_PTBD4_MASK,0x02
		//MOV    #0x00,0x02
		STA    0x02
		PULA
		BRA    End1
	On1:
		MOV    #PTBD_PTBD4_MASK,0x02
		NOP    
		NOP    
		MOV    #0x00,0x02
	End1:
		DECX
		BNE    Loop1
		
	// Blue
	// Pop 1st param from stack
		AIS    #2
		PULA
		AIS    #-3

		LDX    #8
	Loop3:
		ASRA
		BCS    On3
	Off3:
		PSHA
		LDA    #0
		MOV    #PTBD_PTBD4_MASK,0x02
		//MOV    #0x00,0x02
		STA    0x02
		PULA
		BRA    End3
	On3:
		MOV    #PTBD_PTBD4_MASK,0x02
		NOP    
		NOP    
		MOV    #0x00,0x02
	End3:
		DECX
		BNE    Loop3	
	}
}
