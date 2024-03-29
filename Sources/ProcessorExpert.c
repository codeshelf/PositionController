/*
 * Codeshelf
 * � Copyright 2005-2013 Jeffrey B. Williams
 * All rights reserved
 * 
 * $Id$
 * $Name$
 */

#include "Cpu.h"
#include "Events.h"
#include "KBI.h"
#include "Uart.h"
#include "IIC.h"
#include "Rs485Dir.h"
#include "LedDriverReset.h"
#include "DebounceTimer.h"
#include "Flash.h"
#include "ConfigModeWait.h"
#include "AckButtonDelay.h"
#include "Wait.h"
#include "StatusLedClkDo.h"
#include "TPM1.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "StatusLed.h"
#include "SerialBus.h"
#include "Commands.h"
#include "Display.h"

extern uint8_t gLedRedValue;
extern uint8_t gLedGreenValue;
extern uint8_t gLedBlueValue;
extern uint8_t gLedLightStyle;
void main(void) {
	FrameDataType frame[MAX_FRAME_BYTES];
	FrameCntType frameSize;

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
	/*** End of Processor Expert internal initialization.                    ***/
		
#ifdef __IIC
	Init_I2CM();
#endif
	
#ifdef __SharpDisplay
	SPIC2_SPC0 = 0x01;
	SPIC2_BIDIROE = 0x01;
#endif
	
	Wait_Waitms(20);
	initDisplay();
	RS485_RX;

	for (;;) {
		// Read a frame from the serial bus and then process it.
		frameSize = serialReceiveFrame(frame, MAX_FRAME_BYTES);
		if (frameSize > 0) {
			processFrame(frame, frameSize);
		}
	}

	/*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/
