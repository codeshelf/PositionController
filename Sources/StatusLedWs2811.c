/*
 * StatusLedWs2811.c
 *
 *  Created on: Apr 4, 2015
 *      Author: jeffw
 */

#include "StatusLed.h"
#include "IO_Map.h"
#include "StatusLedClkDo.h"
#include "Wait.h"

#define		ON		\
		asm			MOV #0b00001000, 2;	\
		asm			NOP;				\
		asm			NOP;				\
		asm			MOV #0b00000000, 2;

#define		OFF		\
		asm			MOV #0b00001000, 2;	\
		asm			MOV #0b00000000, 2;	

void setStatusLed(uint8_t red, uint8_t green, uint8_t blue) {

asm {
	// Param 1 (red) on stack
	// Param 2 (green) in X
	// Param 3 (blue) in A
	
	// Push 3rd param
		PSHA
		
	// Green
	// Transfer X (green) into A
		TXA
		LDX		#8
	Loop2:
		ASLA
		BCS		On2
	Off2:
		BSET	4, _PTBD
		NOP
		NOP
		BCLR    4, _PTBD
		BRA		End2
	On2:
		BSET	4, _PTBD
		NOP    
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		BCLR    4, _PTBD
	End2:
		DECX
		BNE		Loop2
	
	// Red
	// Pop 1st param from stack
		AIS    #6
		PULA
		AIS    #-7

		LDX    #8
	Loop3:
		ASLA
		BCS    On3
	Off3:
		BSET	4, _PTBD
		NOP
		NOP
		BCLR    4, _PTBD
		BRA    End3
	On3:
		BSET	4, _PTBD
		NOP    
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		BCLR    4, _PTBD
	End3:
		DECX
		BNE    Loop3	

	// Blue
	// The third parameter was pushed onto the stack
		PULA
		
		LDX    #8
	Loop1:
		ASLA
		BCS    On1
	Off1:
		BSET	4, _PTBD
		NOP
		NOP
		BCLR    4, _PTBD
		BRA    End1
	On1:
		BSET	4, _PTBD
		NOP    
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		BCLR    4, _PTBD
	End1:
		DECX
		BNE    Loop1
		
	}
}
