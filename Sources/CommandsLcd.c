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
#include "Display.h"


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

void initDisplay() {
	clearDisplay();
	displayString(20, 10,  "32C", 2);
	displayString(2, 100, "PACK LIGHT", 1);
	displayBarcode(20, 50, "*01*", 1);
}


// --------------------------------------------------------------------------

void clearDisplay() {
	clearLcdDisplay();
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
}

// --------------------------------------------------------------------------

void displayValue(uint8_t displayValue) {
}

void displayValueAsCode(uint8_t controlValue) {
}

// --------------------------------------------------------------------------

void setLedSegments(uint8_t* displayBytesPtr, uint8_t firstDigitVal, uint8_t secondDigitVal) {
}

// --------------------------------------------------------------------------

void displayValueBlink(uint8_t displayValue) {
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
